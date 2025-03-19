#pragma once

#include "FreeFlyCamera.hpp"

#include <glm/vec3.hpp>
#include <klein/klein.hpp>

class Player
{
public:
  glm::FreeflyCamera camera{{position.x(), position.y(), position.z()}};
  Player(const kln::point &_pos) : position{_pos} {}

  void moveUp(float speed);
  void moveLeft(float speed);
  void jump();
  void update();
  const glm::vec3 getPos() const;

private:
  kln::point position;
  kln::direction velocity;
  float gravity = -9.81f;
  float jumpStrength = 5.0f;
  bool isGrounded;
};