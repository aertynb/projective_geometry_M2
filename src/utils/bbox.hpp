#pragma once

#include <glm/vec3.hpp>
#include <klein/klein.hpp>
#include <utility>
#include <vector>

namespace kln
{
class Transformation
{
  point _pos{};
  rotor _r{};
  glm::vec3 _scale{1};

  const std::vector<plane> getPlanes() const
  {
    std::vector<plane> planes(6);
    float scale = _scale.x;
    kln::plane localPlanes[6] = {
        {1 + _pos.x(), 0, 0, 0},  // +X
        {-1 + _pos.x(), 0, 0, 0}, // -X
        {0, 1 + _pos.y(), 0, 0},  // +Y
        {0, -1 + _pos.y(), 0, 0}, // -Y
        {0, 0, 1 + _pos.z(), 0},  // +Z
        {0, 0, -1 + _pos.z(), 0}  // -Z
    };

    // Transform each plane by applying the rotation and translation
    for (int i = 0; i < 6; i++) {
      kln::plane rotatedPlane = _r(localPlanes[i]); // Rotate normal
      // rotatedPlane. -= rotatedPlane ^ _pos;      // Adjust plane offset
      planes[i] = localPlanes[i];
    }
    return planes;
  };

public:
  Transformation(const point &pos, const rotor &r, const glm::vec3 &scale) :
      _pos{pos}, _r{r}, _scale{scale}
  {
  }

  Transformation(const point &pos) : _pos{pos} {}

  bool collidesWith(const point &playerPos)
  {
    const auto planes = getPlanes();
    for (const auto &plane : planes) {
      auto d = playerPos ^ plane;
      if (d.q >= 1) {
        return false;
      }
    }

    return true;
  }
};
} // namespace kln
