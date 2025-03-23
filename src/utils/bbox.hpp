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

public:
  Transformation(const point &pos, const rotor &r, const glm::vec3 &scale) :
      _pos{pos}, _r{r}, _scale{scale}
  {
  }

  Transformation(const point &pos) : _pos{pos} {}

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

  bool collidesWith(const point &targetPos) const
  {
    const auto planes = getPlanes();
    for (const auto &plane : planes) {
      // const auto plane = planes[5];
      auto d = targetPos ^ plane;
      // std::cout << d.q << std::endl;
      if (d.q >= 0) {
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

  glm::vec3 getDirection(const kln::line &line) const
  {
    return glm::normalize(glm::vec3(line.e23(), line.e31(), line.e12()));
  }

  bool computeColliderCoords(
      const kln::point &start, const kln::point &end, glm::vec3 &output) const
  {
    auto line = end & start;
    for (const auto &transfo : transfos) {
      const auto planes = transfo.getPlanes();
      const auto plane = planes[0];
      // for (const auto plane : planes) {
      auto point = line ^ plane;
      std::cout << plane.x() << ", " << plane.y() << ", " << plane.z() << ", "
                << plane.d() << std::endl;
      output = glm::vec3(point.x(), point.y(), point.z());
      return true;
      // if (transfo.collidesWith(point)) {
      //   output = glm::vec3(point.x(), point.y(), point.z());
      //   return true;
      // }
      // }
    }
    return false;
  }

  bool findIntersection(const kln::point &start, const glm::vec3 &direction,
      float maxDistance, glm::vec3 &output) const
  {
    float stepSize = 0.1f; // Step size for checking (adjust for precision)
    int nbPlanes = 6;
    const auto T =
        kln::translator(stepSize, direction.x, direction.y, direction.z);
    for (const auto &transfo : transfos) {
      auto temp = start;
      for (float t = 0; t <= maxDistance; t += stepSize) {
        // Generate new point along the direction
        kln::point samplePoint = T(temp);

        // If intersection is valid (negative sign means in front)
        if (transfo.collidesWith(samplePoint)) {
          output = glm::vec3(samplePoint.x(), samplePoint.y(), samplePoint.z());
          // std::cout << samplePoint.x() << ", " << samplePoint.y() << ", "
          //           << samplePoint.z() << std::endl;
          return true;
        }

        temp = samplePoint;
      }
    }
    return false; // No intersection found within range
  }
};
} // namespace kln