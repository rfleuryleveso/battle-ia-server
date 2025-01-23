#ifndef WORLD_PAWN_HPP
#define WORLD_PAWN_HPP
#include "world/vector3.hpp"
#include "world/world_object.hpp"
#include <chrono>
#include <cstdint>

namespace World {
class Pawn : public WorldObject {
private:
  uint8_t health_ = 100;
  uint8_t armor_ = 0;
  int score_ = 0;
  Vector3 target_speed_ = Vector3(0.0f, 0.0f, 0.0f);

  /**
   * Last time the pawn shot
   */
  std::chrono::time_point<std::chrono::steady_clock> last_shoot =
      std::chrono::steady_clock::time_point::min();

public:
  Pawn() {}
  ~Pawn() override { spdlog::info("Pawn destroyed {}", this->GetId()); };

  void HandleCollision(std::shared_ptr<WorldObject> other) override;

  bool HandleDamage(uint8_t damage);

  void SetHealth(uint8_t health) { health_ = health; };
  uint8_t GetHealth() { return health_; };

  void AddScore(int score) { score_ += score; };
  int GetScore() { return score_; };

  void AddArmor(int armor) { armor_ += armor; }
  uint8_t GetArmor() { return armor_; };

  void Tick() override;

  Vector3 &GetTargetSpeed() { return target_speed_; };

  WorldObjectType GetWorldObjectType() override {
    return WorldObjectType::PAWN;
  }

  /*! \brief Register a shoot in the Pawn's memory. Does not actually does the
   * shoot, as it is handled by the executor
   * \returns true if the shoot is allowed
   */
  bool RegisterShoot();
};

} // namespace World

#endif
