#ifndef WORLD_WALL_HPP
#define WORLD_WALL_HPP
#include "world/world_object.hpp"
#include <cstdint>

namespace World {
class Wall : public WorldObject {

public:
  Wall() {}
  virtual ~Wall() = default;

  void HandleCollision(std::shared_ptr<WorldObject> other) override {
    if (other->GetWorldObjectType() == WorldObjectType::PAWN) {
      auto other_object_speed = other->GetSpeed();
      other_object_speed.Zero();
    }
  };

  WorldObjectType GetWorldObjectType() override {
    return WorldObjectType::WALL;
  }
};

} // namespace World

#endif
