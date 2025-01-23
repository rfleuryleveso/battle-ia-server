#include "world/world_object.hpp"
#include <cstdint>
#include <random>

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type>
    random_distr(1, UINT64_MAX);
namespace World {
WorldObject::WorldObject() { this->id_ = random_distr(rng); }
void WorldObject::HandleCollision(std::shared_ptr<WorldObject> other) {}
} // namespace World
