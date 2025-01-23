#ifndef WORLD_BOOST_HPP
#define WORLD_BOOST_HPP
#include "world/world_object.hpp"
#include <cstdint>

namespace World {
class Boost : public WorldObject {

public:
  Boost() {}
  virtual ~Boost() = default;

  void HandleCollision(std::shared_ptr<WorldObject> other) override;

  bool HandleDamage(uint8_t damage);

  WorldObjectType GetWorldObjectType() override {
    return WorldObjectType::PAWN;
  }
};

} // namespace World

#endif
