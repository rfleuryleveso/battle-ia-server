

#ifndef VISUALIZER_RAYLIBVISUALIZER_HPP
#define VISUALIZER_RAYLIBVISUALIZER_HPP

#include "visualizer/visualizer.hpp"
#include "world/world.hpp"

#include <memory>

class RaylibVisualizer : public std::enable_shared_from_this<RaylibVisualizer>,
                         public Visualizer {
private:
  using Shoot = std::pair<World::Vector3, World::Vector3>;
  using TimedShoot = std::pair<Shoot, std::chrono::steady_clock::time_point>;

  const int screen_width_ = 800;
  const int screen_height_ = 800;
  std::mutex shoot_mutex_; // Protect shared access to shoots
  std::vector<TimedShoot>
      shoots_; // Store active shoots with timestamps  void Process();

  void DrawWorldObjects();

  void Start() override;

  void RemoveExpiredShoots();

  void Process();

public:
  RaylibVisualizer(std::shared_ptr<World::World> world) : Visualizer(world) {}

  void DrawShoot(World::Vector3 start, World::Vector3 end) override;
};

#endif // VISUALIZER_RAYLIBVISUALIZER_HPP
