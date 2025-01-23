#include "world/world.hpp"
#include "constants.hpp"
#include "world/boost.hpp"
#include "world/pawn.hpp"
#include "world/vector3.hpp"
#include "world/wall.hpp"
#include "world/world_object.hpp"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <random>
#include <spdlog/spdlog.h>
#include <thread>

namespace World {
void World::AddObject(std::shared_ptr<WorldObject> wo) {
  std::lock_guard<std::mutex> lock(wo_m);

  this->world_objects_.push_back(wo);
}

std::vector<std::shared_ptr<WorldObject>> &World::GetWorldObjects() {
  return world_objects_;
}

void World::Start() {
  this->is_running_ = true;
  this->process_thread_ = std::thread(&World::Process, this);
  spdlog::info("World started");
}
void World::Stop() {
  spdlog::info("Stopping the world.");
  this->is_running_ = false;
  if (this->process_thread_.joinable()) {
    this->process_thread_.join();
  }
  spdlog::info("World stopped");
}

void World::Process() {
  using namespace std::chrono;

  const int target_fps = 60;
  const double delta_time = (double)1 / (double)target_fps;
  ;
  // Define the time per frame for 60 FPS
  const milliseconds frameDuration(1000 / target_fps);

  auto lastTime = steady_clock::now();

  while (this->is_running_) {
    auto currentTime = steady_clock::now();
    auto elapsedTime = duration_cast<milliseconds>(currentTime - lastTime);

    if (elapsedTime >= frameDuration) {
      lastTime += frameDuration;

      {
        std::lock_guard<std::mutex> lock(wo_m);
        auto wos = this->GetWorldObjects();
        for (auto &world_object : wos) {
          if (world_object->IsDestroyed()) {
            continue;
          }
          try {
            world_object->Tick();
          } catch (std::exception e) {
            spdlog::error("Could not tick {} {}", world_object->GetId(),
                          e.what());
          }
          Vector3 &position = world_object->GetPosition();
          Vector3 &speed = world_object->GetSpeed();
          Vector3 new_position(position.GetX() + speed.GetX() * delta_time,
                               position.GetY() + speed.GetY() * delta_time,
                               position.GetZ() + speed.GetZ() * delta_time);
          // OOB Check
          if (new_position.GetX() > this->world_settings_.sizeX_ ||
              new_position.GetX() < 0 ||
              new_position.GetY() > this->world_settings_.sizeY_ ||
              new_position.GetY() < 0) {
            if (speed.GetX() > EPSILON) {
              speed.SetX(0);
              speed.SetY(0);
            }
            continue;
          }

          {
            // Collision check
            for (auto &world_object_2 : wos) {
              if (world_object_2->IsDestroyed()) {
                continue;
              }
              Vector3 &wo_2_position = world_object_2->GetPosition();

              int distance = std::sqrt(
                  std::pow(wo_2_position.GetX() - new_position.GetX(), 2) +
                  std::pow(wo_2_position.GetY() - new_position.GetY(), 2));

              if (world_object_2->GetRadius() != 0 &&
                  world_object->GetRadius() != 0 &&
                  distance < (world_object_2->GetRadius() +
                              world_object->GetRadius())) {
                // Collision detected!
                world_object_2->HandleCollision(world_object);
                world_object->HandleCollision(world_object_2);
                break;
              }
            }
          }

          // Update position
          position.SetX(new_position.GetX());
          position.SetY(new_position.GetY());
          position.SetZ(new_position.GetZ());
        }
      }

      // Skip additional frames if the system is lagging
      while (elapsedTime >= frameDuration) {
        lastTime += frameDuration;
        elapsedTime -= frameDuration;
      }
    }

    // Sleep to prevent busy-waiting (ensure loop doesn't consume 100% CPU)
    std::this_thread::sleep_for(milliseconds(1));
  }
}
std::shared_ptr<WorldObject> World::GetWorldObjectById(uint64_t id) {
  for (auto &wo : this->world_objects_) {
    if (wo->GetId() == id) {
      return wo;
    }
  }
  return nullptr;
}

void World::GenerateRandomWalls(int num_walls, int wall_radius) {
  // Seed the random number generator
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_real_distribution<double> x_dist(0.0,
                                                this->world_settings_.sizeX_);
  std::uniform_real_distribution<double> y_dist(0.0,
                                                this->world_settings_.sizeY_);

  for (int i = 0; i < num_walls; ++i) {
    // Generate random x and y coordinates
    double x = x_dist(rng);
    double y = y_dist(rng);

    // Create a new wall and set its properties
    auto wall = std::make_shared<Wall>();
    wall->GetPosition().SetX(x);
    wall->GetPosition().SetY(y);
    wall->SetRadius(wall_radius); // Set radius for visualization

    // Add the wall to the world
    AddObject(wall);
    std::cout << "Wall created at (" << x << ", " << y << ")" << std::endl;
  }
}

void World::GenerateRandomBoosts(int num_boosts) {
  // Seed the random number generator
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_real_distribution<double> x_dist(0.0,
                                                this->world_settings_.sizeX_);
  std::uniform_real_distribution<double> y_dist(0.0,
                                                this->world_settings_.sizeY_);

  for (int i = 0; i < num_boosts; ++i) {
    // Generate random x and y coordinates
    double x = x_dist(rng);
    double y = y_dist(rng);

    auto boost = std::make_shared<Boost>();
    boost->GetPosition().SetX(x);
    boost->GetPosition().SetY(y);
    boost->SetRadius(1); // Set radius for visualization

    AddObject(boost);
    std::cout << "Boost created at (" << x << ", " << y << ")" << std::endl;
  }
}
void World::GenWallBounds() {
  for (size_t x = 0; x < this->world_settings_.sizeX_; x++) {

    auto wall = std::make_shared<Wall>();
    wall->GetPosition().SetX(x);
    wall->GetPosition().SetY(1);
    wall->SetRadius(1);

    AddObject(wall);

    wall = std::make_shared<Wall>();
    wall->GetPosition().SetX(x);
    wall->GetPosition().SetY(this->world_settings_.sizeY_ - 1);
    wall->SetRadius(1);

    AddObject(wall);
  }
  for (size_t y = 0; y < this->world_settings_.sizeY_; y++) {
    auto wall = std::make_shared<Wall>();
    wall->GetPosition().SetX(1);
    wall->GetPosition().SetY(y);
    wall->SetRadius(1);

    AddObject(wall);

    wall = std::make_shared<Wall>();
    wall->GetPosition().SetX(this->world_settings_.sizeX_ - 1);
    wall->GetPosition().SetY(y);
    wall->SetRadius(1);

    AddObject(wall);
  }
}
} // namespace World
