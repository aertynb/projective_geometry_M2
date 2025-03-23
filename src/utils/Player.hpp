#pragma once

#include "FreeFlyCamera.hpp"
#include "bbox.hpp"
#include "line.hpp"

#include <glm/vec3.hpp>
#include <klein/klein.hpp>

#include <iostream>

class Player
{
public:
  glm::FreeflyCamera camera{{position.x(), position.y() + 0.5f, position.z()}};
  Player(const kln::point &_pos, kln::Bbox &_bbox) : position{_pos}, bbox{_bbox}
  {
  }

  void moveUp(float speed);
  void moveLeft(float speed);
  void jump();
  void update();
  void drawLine(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix,
      UniformHandler handler) const;
  const glm::vec3 getPos() const;

  kln::point position;
  LineCustom line;

private:
  void applyGravity();

  kln::direction velocity{0.f, 0.f, 0.f};
  float gravity = -2.0f;
  float jumpStrength = 2.0f;
  float verticalVelocity = 0.f;
  float deltaTime = 0.016f;
  bool isGrounded = false;
  kln::translator forwardT;
  kln::translator leftT;
  kln::translator vertT;
  kln::Bbox &bbox;
};