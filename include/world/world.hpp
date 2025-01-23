#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include "world/world_object.hpp"
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

namespace World {

struct WorldSettings {
  double sizeX_ = 100;
  double sizeY_ = 100;

  bool autoaim_enabled_ = false;
  bool radar_enabled_ = true;
  bool max_players_ = 32;
};

class World : std::enable_shared_from_this<World> {
private:
  std::mutex wo_m;
  std::vector<std::shared_ptr<WorldObject>> world_objects_;
  std::thread process_thread_;
  WorldSettings world_settings_;

  bool is_running_ = false;

  void Process();

public:
  void AddObject(std::shared_ptr<WorldObject> wo);

  std::vector<std::shared_ptr<WorldObject>> &GetWorldObjects();

  void Start();

  void Stop();

  double GetSizeX() { return world_settings_.sizeX_; };
  double GetSizeY() { return world_settings_.sizeY_; };

  std::shared_ptr<WorldObject> GetWorldObjectById(uint64_t id);
  void GenerateRandomWalls(int num_walls, int wall_radius);
  void GenWallBounds();
  void GenerateRandomBoosts(int num_boosts);
};
} // namespace World

#endif
