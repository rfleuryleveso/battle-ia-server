#include "world/boost.hpp"
#include "world/pawn.hpp"
#include "world/world_object.hpp"
#include <memory>

namespace World {
void Boost::HandleCollision(std::shared_ptr<WorldObject> other) {
  if (other->GetWorldObjectType() == WorldObjectType::PAWN) {
    std::shared_ptr<Pawn> pawn = std::dynamic_pointer_cast<Pawn>(other);
    pawn->AddScore(30);
    pawn->AddArmor(50);
    this->SetIsDestroyed(true);
  }
}

bool Boost::HandleDamage(uint8_t damage) {}
} // namespace World
