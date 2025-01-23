#ifndef EXECUTOR_EXECUTOR_HPP
#define EXECUTOR_EXECUTOR_HPP

#include "server/peer.hpp"
#include "server/server.hpp"
#include "visualizer/visualizer.hpp"
#include "world/pawn.hpp"
#include "world/world.hpp"

#include <map>
#include <memory>
#include <spdlog/spdlog.h>

class Executor : public std::enable_shared_from_this<Executor> {
private:
  std::shared_ptr<Server> server_;
  std::shared_ptr<World::World> world_;
  std::shared_ptr<Visualizer> visualizer_;
  std::map<std::shared_ptr<Peer>, std::shared_ptr<World::Pawn>> peer_to_pawns_;

  void ProcessPeer(const std::shared_ptr<Peer> &peer);

  void SendPlayerData(const std::shared_ptr<Peer> &peer,
                      const std::shared_ptr<World::Pawn> &pawn);
  void HandleWorldInfoRequest(const std::shared_ptr<Peer> &peer);
  void HandleSetSpeed(const std::shared_ptr<Peer> &peer,
                      const ClientServerMessage &message,
                      const std::shared_ptr<World::Pawn> &pawn);
  void HandleRadarPing(const std::shared_ptr<Peer> &peer);
  void HandleShoot(const std::shared_ptr<Peer> &peer,
                   const ClientServerMessage &message);

public:
  Executor(std::shared_ptr<Server> server, std::shared_ptr<World::World> world,
           std::shared_ptr<Visualizer> visualizer)
      : server_(std::move(server)), world_(std::move(world)),
        visualizer_(std::move(visualizer)) {}

  ~Executor() { spdlog::info("Stopping executor"); }

  void Process();
  bool ProcessFrame();

  template <typename Duration> void ProcessForDuration(Duration duration) {
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + duration;
    spdlog::info(
        "Processing world for {} seconds",
        std::chrono::duration_cast<std::chrono::duration<float>>(duration)
            .count());
    while (std::chrono::steady_clock::now() < end_time) {
      if (!this->ProcessFrame()) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    spdlog::info("Processing ended");
    this->BroadcastGameEnded();
  }

  void BroadcastGameEnded();

  bool IsPeerAlive(const std::shared_ptr<Peer> &peer);
};

#endif // EXECUTOR_EXECUTOR_HPP
