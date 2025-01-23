
#include "executor/executor.hpp"
#include "battle_c.pb.h"
#include "server/peer.hpp"
#include "world/pawn.hpp"
#include "world/world_object.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <random>
#include <spdlog/spdlog.h>
#include <thread>

void Executor::Process() {
  std::cout << "Executor, processing" << std::endl;
  while (true) {
    if (!this->ProcessFrame()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  this->BroadcastGameEnded();
}

bool Executor::ProcessFrame() {
  uint64_t alive_pawns = 0;
  uint64_t total_pawns = 0;
  for (const auto &peer : server_->GetPeers()) {
    if (peer->GetPeerType() == PeerType::PAWN && !peer->IsDead()) {
      try {
        ProcessPeer(peer);
      } catch (std::exception e) {
        peer->SetIsDead(true);
      }
      total_pawns++;
      alive_pawns += this->IsPeerAlive(peer) ? 1 : 0;
    }
  }
  if (total_pawns >= 2 && alive_pawns == 1) {
    spdlog::info("Game has ended ! All but 1 pawns are dead");
    return false;
  }

  return true;
}
bool Executor::IsPeerAlive(const std::shared_ptr<Peer> &peer) {
  auto &pawn = peer_to_pawns_[peer];

  if (!pawn) {
    return false;
  }

  return !pawn->IsDestroyed();
}
void Executor::ProcessPeer(const std::shared_ptr<Peer> &peer) {
  auto &pawn = peer_to_pawns_[peer];

  if (!pawn) {
    pawn = std::make_shared<World::Pawn>();
    // Seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<double> x_dist(0, world_->GetSizeX());
    std::uniform_real_distribution<double> y_dist(0, world_->GetSizeY());

    auto &position = pawn->GetPosition();
    position.SetX(x_dist(rng));
    position.SetY(y_dist(rng));

    peer_to_pawns_[peer] = pawn;
    world_->AddObject(std::static_pointer_cast<World::WorldObject>(pawn));
    spdlog::info("Spawning pawn id = {}, x = {}, y = {}", pawn->GetId(),
                 pawn->GetPosition().GetX(), pawn->GetPosition().GetY());

    ServerClientMessage message;
    message.mutable_game_started();
    peer->QueueMessage(message);
    spdlog::info("Player spawned and notified");
  }

  SendPlayerData(peer, pawn);

  auto client_message = peer->PopMessage();
  if (!client_message.has_value()) {
    return;
  }
  if (client_message->get_world_info()) {
    HandleWorldInfoRequest(peer);
  } else if (client_message->has_set_speed()) {
    HandleSetSpeed(peer, *client_message, pawn);
  } else if (client_message->has_radar_ping()) {
    HandleRadarPing(peer);
  } else if (client_message->has_shoot()) {
    HandleShoot(peer, *client_message);
  }
}

void Executor::SendPlayerData(const std::shared_ptr<Peer> &peer,
                              const std::shared_ptr<World::Pawn> &pawn) {
  ServerClientMessage message;
  PlayerData *pd = new PlayerData();

  auto position = pawn->GetPosition();
  auto speed = pawn->GetSpeed();

  Vector3 *position_message = new Vector3();
  position_message->set_x(position.GetX());
  position_message->set_y(position.GetY());
  position_message->set_z(position.GetZ());
  pd->set_allocated_position(position_message);

  Vector3 *speed_message = new Vector3();
  speed_message->set_x(speed.GetX());
  speed_message->set_y(speed.GetY());
  speed_message->set_z(speed.GetZ());
  pd->set_allocated_speed(speed_message);

  pd->set_id(pawn->GetId());
  pd->set_armor(pawn->GetArmor());
  pd->set_health(pawn->GetHealth());
  pd->set_score(pawn->GetScore());
  pd->set_alive(!pawn->IsDestroyed());

  message.set_allocated_player_data(pd);

  peer->QueueMessage(message);
}

void Executor::HandleWorldInfoRequest(const std::shared_ptr<Peer> &peer) {
  ServerClientMessage message;
  WorldOptions *wo = new WorldOptions();
  wo->set_map_x(world_->GetSizeX());
  wo->set_map_y(world_->GetSizeY());
  wo->set_auto_shoot_allowed(false);
  wo->set_grid_based(false);
  wo->set_max_players(32);
  wo->set_radar_enabled(true);
  message.set_allocated_world_options(wo);

  peer->QueueMessage(message);
}

void Executor::HandleSetSpeed(const std::shared_ptr<Peer> &peer,
                              const ClientServerMessage &message,
                              const std::shared_ptr<World::Pawn> &pawn) {
  auto set_speed_message = message.set_speed();
  auto &target_speed = pawn->GetTargetSpeed();
  target_speed.SetX(set_speed_message.x());
  target_speed.SetY(set_speed_message.y());
  target_speed.SetZ(set_speed_message.z());
}

void Executor::HandleRadarPing(const std::shared_ptr<Peer> &peer) {
  ServerClientMessage message;
  RadarResult *radar_result = new RadarResult();

  for (const auto &obj : world_->GetWorldObjects()) {
    if (obj->GetWorldObjectType() == World::WorldObjectType::UNKNOWN) {
      continue;
    }

    RadarReturn *radar_return = radar_result->add_radar_return();
    radar_return->set_id(obj->GetId());

    Vector3 *position = new Vector3();
    position->set_x(obj->GetPosition().GetX());
    position->set_y(obj->GetPosition().GetY());
    position->set_z(obj->GetPosition().GetZ());
    radar_return->set_allocated_position(position);

    Vector3 *speed = new Vector3();
    speed->set_x(obj->GetSpeed().GetX());
    speed->set_y(obj->GetSpeed().GetY());
    speed->set_z(obj->GetSpeed().GetZ());
    radar_return->set_allocated_speed(speed);

    radar_return->set_return_type(
        obj->GetWorldObjectType() == World::WorldObjectType::PAWN
            ? ::RadarReturnType::PLAYER
        : obj->GetWorldObjectType() == World::WorldObjectType::BOOST
            ? ::RadarReturnType::BOOST
            : ::RadarReturnType::WALL);
  }

  message.set_allocated_radar_result(radar_result);
  peer->QueueMessage(message);
}

void Executor::HandleShoot(const std::shared_ptr<Peer> &peer,
                           const ClientServerMessage &message) {
  auto shoot_message = message.shoot();
  std::shared_ptr<World::WorldObject> target;

  ShootResult *shoot_result_message = new ShootResult();

  auto &pawn = peer_to_pawns_[peer];

  if (!pawn) {
    return;
  }

  if (!pawn->RegisterShoot()) {
    spdlog::warn("Shot denied for pawn ID {}", pawn->GetId());
    shoot_result_message->set_success(false);
    shoot_result_message->set_fail_reason(ShootFailReason::COOLDOWN);
    goto send_response;
  }

  spdlog::info("Received shoot request from pawn ID: {}", pawn->GetId());
  if (shoot_message.has_target_id()) {
    target = world_->GetWorldObjectById(shoot_message.target_id());
    if (target &&
        target->GetWorldObjectType() == World::WorldObjectType::PAWN) {
      shoot_result_message->set_target_id(shoot_message.target_id());
      shoot_result_message->set_success(true);
      shoot_result_message->set_damage_points(50);

      auto target_pawn = std::static_pointer_cast<World::Pawn>(target);

      if (target_pawn->HandleDamage(50)) {
        shoot_result_message->set_target_destroyed(true);
      }
    }
  } else if (shoot_message.has_angle()) {
    // Handle shooting by angle
    auto shooter_position = pawn->GetPosition();
    double angle = shoot_message.angle();

    spdlog::info("Peer ID: {} shooting at angle: {}", pawn->GetId(),
                 shoot_message.angle());
    World::Vector3 direction((double)std::cos(angle), (double)std::sin(angle),
                             (double)0.0f);

    // Simulate ray tracing to find the first target hit
    double max_range = 1000.0; // Maximum shooting range
    double step = 1.0;         // Step size for ray trace
    World::Vector3 current_position = shooter_position;

    for (double distance = 0.0; distance < max_range; distance += step) {
      current_position.SetX(current_position.GetX() + direction.GetX() * step);
      current_position.SetY(current_position.GetY() + direction.GetY() * step);

      // Check for collisions with objects
      for (auto &world_object : world_->GetWorldObjects()) {
        if (!world_object || world_object->IsDestroyed() ||
            world_object->GetId() == pawn->GetId()) {
          continue;
        }

        // Calculate distance to the object
        auto object_position = world_object->GetPosition();
        double object_distance = std::sqrt(
            std::pow(object_position.GetX() - current_position.GetX(), 2) +
            std::pow(object_position.GetY() - current_position.GetY(), 2));

        if (object_distance <= world_object->GetRadius()) {
          target = world_object;
          break;
        }
      }

      if (target) {
        break;
      }
    }

    if (target) {
      shoot_result_message->set_target_id(target->GetId());
      shoot_result_message->set_success(true);
      shoot_result_message->set_damage_points(50);

      if (target->GetWorldObjectType() == World::WorldObjectType::PAWN) {
        pawn->AddScore(20);
        auto target_pawn = std::static_pointer_cast<World::Pawn>(target);

        if (target_pawn->HandleDamage(50)) {
          pawn->AddScore(50);
          shoot_result_message->set_target_destroyed(true);
        }
      }
      this->visualizer_->DrawShoot(pawn->GetPosition(), target->GetPosition());
    } else {
      shoot_result_message->set_success(false); // No target hit
      shoot_result_message->set_fail_reason(ShootFailReason::MISS);
    }
  }

send_response:
  ServerClientMessage response_message;
  response_message.set_allocated_shoot_result(shoot_result_message);
  peer->QueueMessage(response_message);
}

void Executor::BroadcastGameEnded() {
  for (const auto &peer : server_->GetPeers()) {
    if (peer->GetPeerType() == PeerType::PAWN) {

      ServerClientMessage message;
      message.mutable_game_ended()->set_reason(GameEndedReason::GAME_STOPPED);
      peer->QueueMessage(message);
    }
  }
  spdlog::info("Broadcasted game_ended");
}
