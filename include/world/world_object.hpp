#ifndef WORLD_WORLD_OBJECT_HPP
#define WORLD_WORLD_OBJECT_HPP

#include "vector3.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>
#include <sys/types.h>

namespace World {

enum class WorldObjectType { PAWN, WALL, BOOST, UNKNOWN };

class WorldObject {
  uint64_t id_;
  Vector3 position_ = Vector3(0.0f, 0.0f, 0.0f);
  Vector3 speed_ = Vector3(0.0f, 0.0f, 0.0f);
  Vector3 acceleration_ = Vector3(0.0f, 0.0f, 0.0f);

  uint64_t radius_ = 1;

  bool destroyed_;

public:
  WorldObject();
  virtual ~WorldObject() { spdlog::trace("Destroyed WO {}", this->GetId()); };
  virtual void Tick() {};

  uint64_t GetId() { return id_; }
  Vector3 &GetPosition() { return position_; }
  Vector3 &GetSpeed() { return speed_; }
  Vector3 &GetAcceleration() { return acceleration_; }
  uint64_t GetRadius() { return radius_; }
  void SetRadius(uint64_t radius) { radius_ = radius; }

  virtual void HandleCollision(std::shared_ptr<WorldObject> other);

  bool IsDestroyed() { return destroyed_; }

  void SetIsDestroyed(bool destroyed) { destroyed_ = destroyed; }

  virtual WorldObjectType GetWorldObjectType() {
    return WorldObjectType::UNKNOWN;
  }
};
} // namespace World
#endif
