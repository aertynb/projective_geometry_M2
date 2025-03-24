#pragma once

#include "FreeFlyCamera.hpp"
#include "bbox.hpp"
#include "line.hpp"

#include <algorithm>
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
  void createLine();
  void clearLine();
  const glm::vec3 getPos() const;

  kln::point position;
  LineCustom line;

private:
  void applyGravity();
  float maxSpeed = 3.0f;
  float currentSpeed = 0.f;
  float gravity = -9.81f;
  float jumpStrength = 5.0f;
  float verticalVelocity = 0.f;
  float deltaTime = 0.032f;
  bool isGrounded = false, isHooked = false;
  kln::translator forwardT;
  kln::translator leftT;
  kln::translator vertT;
  kln::Bbox &bbox;
  glm::vec3 velocity{0.0f};
};