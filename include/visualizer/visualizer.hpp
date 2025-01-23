

#ifndef VISUALIZER_VISUALIZER_HPP
#define VISUALIZER_VISUALIZER_HPP

#include "world/vector3.hpp"
#include "world/world.hpp"

#include <atomic>
#include <memory>
#include <thread>

class Visualizer {
protected:
  std::shared_ptr<World::World> world_;
  std::thread visualizer_thread_;
  std::atomic<bool> running_;

public:
  Visualizer(std::shared_ptr<World::World> world)
      : world_(std::move(world)), running_(false) {}

  virtual ~Visualizer() {
    if (running_) {
      running_ = false;
      if (visualizer_thread_.joinable()) {
        visualizer_thread_.join();
      }
    }
  }

  virtual void Start() {};

  virtual void DrawShoot(World::Vector3 start, World::Vector3 end) {};
};

#endif // VISUALIZER_VISUALIZER_HPP
