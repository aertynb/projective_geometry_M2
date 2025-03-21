#pragma once

#include <glm/vec3.hpp>
#include <iostream>
#include <klein/klein.hpp>
#include <utility>
#include <vector>

namespace kln
{
class Transformation
{
private:
  point _pos{};
  rotor _r{};
  glm::vec3 _scale{1};

  const std::vector<plane> getPlanes() const
  {
    std::vector<plane> planes(6);
    float scale = _scale.x;
    kln::plane localPlanes[6] = {
        {1, 0, 0, 1 - _pos.x()},  // +X
        {-1, 0, 0, 1 + _pos.x()}, // -X
        {0, 1, 0, 1 - _pos.y()},  // +Y
        {0, -1, 0, 1 + _pos.y()}, // -Y
        {0, 0, 1, 1 - _pos.z()},  // +Z
        {0, 0, -1, 1 + _pos.z()}  // -Z
    };

    // Transform each plane by applying the rotation and translation
    for (int i = 0; i < 6; i++) {
      //   kln::plane rotatedPlane = _r(localPlanes[i]); // Rotate normal
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

  bool collidesWith(const point &targetPos) const
  {
    const auto planes = getPlanes();
    for (const auto &plane : planes) {
      // const auto plane = planes[5];
      auto d = targetPos ^ plane;
      // std::cout << d.q << std::endl;
      if (d.q > 0) {
        return false;
      }
    }

    return true;
  }
};

class Bbox
{
private:
  std::vector<Transformation> transfos;

public:
  void add(const Transformation &&transfo)
  {
    transfos.push_back(std::move(transfo));
  }

  bool globalCollidesWith(const point &targetPos)
  {
    // std::cout << transfos.size() << std::endl;
    for (const auto &transfo : transfos) {
      bool collide = transfo.collidesWith(targetPos);
      if (collide)
        return true;
    }
    return false;
  }
};
} // namespace kln