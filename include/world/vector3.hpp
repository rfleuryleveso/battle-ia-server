#ifndef WORLD_VECTOR3_HPP
#define WORLD_VECTOR3_HPP

#include "battle_c.pb.h"
namespace World {
class Vector3 {
private:
  double x_;
  double y_;
  double z_;

public:
  Vector3(double x, double y, double z) : x_(x), y_(y), z_(z) {};

  Vector3() : Vector3(0, 0, 0) {};

  double GetX() const { return this->x_; }

  double GetY() const { return this->y_; }

  double GetZ() const { return this->z_; }

  void SetX(double x) { this->x_ = x; }

  void SetY(double y) { this->y_ = y; }

  void SetZ(double z) { this->z_ = z; }

  void Zero() { this->x_ = this->y_ = this->z_ = 0; }
};
} // namespace World

#endif
