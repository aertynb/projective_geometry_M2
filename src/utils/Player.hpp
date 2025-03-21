#pragma once

#include "FreeFlyCamera.hpp"

#include <glm/vec3.hpp>
#include <klein/klein.hpp>

#include <iostream>

class Player
{
public:
  kln::point position;
  glm::FreeflyCamera camera{{position.x(), position.y(), position.z()}};
  Player(const kln::point &_pos) : position{_pos} {}

  void moveUp(float speed);
  void moveLeft(float speed);
  void jump();
  void update();
  const glm::vec3 getPos() const;

private:
  void applyGravity();

  kln::direction velocity{0.f, 0.f, 0.f};
  float gravity = -1.0f;
  float jumpStrength = 2.0f;
  float verticalVelocity = 0.f;
  float deltaTime = 0.0320f;
  bool isGrounded = true;
  kln::translator forwardT;
  kln::translator leftT;
  kln::translator jumpT;
};