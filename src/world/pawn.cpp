#include "world/pawn.hpp"
#include "constants.hpp"
#include <chrono>
#include <cstdint>

namespace World {
void Pawn::HandleCollision(std::shared_ptr<WorldObject> other) {}

bool Pawn::HandleDamage(uint8_t damage) {
  int final_health = (int)health_ - (int)damage;
  if (final_health < 0) {
    final_health = 0;
  }
  this->SetHealth(final_health);

  this->SetIsDestroyed(final_health == 0);
  return final_health == 0;
}

void Pawn::Tick() {
  auto &speed = this->GetSpeed();
  auto targetSpeed = this->GetTargetSpeed();
  float rate = 0.5f;

  // Update speed in the X direction
  if (speed.GetX() != targetSpeed.GetX()) {
    double delta = rate * (targetSpeed.GetX() - speed.GetX());
    speed.SetX(std::min(std::max(speed.GetX() + delta, (double)-MAX_SPEED_X),
                        (double)MAX_SPEED_X));
  }

  // Update speed in the Y direction
  if (speed.GetY() != targetSpeed.GetY()) {
    float delta = rate * (targetSpeed.GetY() - speed.GetY());
    speed.SetY(std::min(std::max(speed.GetY() + delta, (double)-MAX_SPEED_Y),
                        (double)MAX_SPEED_Y));
  }

  // Update speed in the Z direction
  if (speed.GetZ() != targetSpeed.GetZ()) {
    float delta = rate * (targetSpeed.GetZ() - speed.GetZ());
    speed.SetZ(std::min(std::max(speed.GetZ() + delta, (double)-MAX_SPEED_Z),
                        (double)MAX_SPEED_Z));
  }
}

bool Pawn::RegisterShoot() {
  using namespace std::chrono_literals;
  if (this->last_shoot + 3s > std::chrono::steady_clock::now()) {
    return false;
  }
  this->last_shoot = std::chrono::steady_clock::now();
  return true;
}
} // namespace World
