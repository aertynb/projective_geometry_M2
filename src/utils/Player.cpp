#include "Player.hpp"

void Player::moveUp(float _speed)
{
  const auto &frontVec = camera.m_FrontVector;
  currentSpeed = maxSpeed * deltaTime;
  velocity.z = currentSpeed;
  if (isHooked)
    currentSpeed = (maxSpeed + 2) * deltaTime;
  forwardT = kln::translator(_speed * currentSpeed, frontVec.x, 0, frontVec.z);
}

void Player::moveLeft(float _speed)
{
  const auto &leftVector = camera.m_LeftVector;
  currentSpeed = maxSpeed * deltaTime;
  velocity.z = currentSpeed;
  leftT = kln::translator(_speed * currentSpeed, leftVector.x, 0, leftVector.z);
}

void Player::jump()
{
  if (isGrounded) {
    verticalVelocity = jumpStrength; // Apply jump force
    isGrounded = false;
  }
}

void Player::applyGravity()
{

  verticalVelocity +=
      gravity * deltaTime; // Gravity decreases velocity over time
  if (isHooked) {
    verticalVelocity = std::clamp(verticalVelocity, -1.0f, 1.0f);
  }
  // std::cout << verticalVelocity << std::endl;
}

void Player::update()
{
  applyGravity();

  // Apply jumping motion (always affects position)
  vertT = kln::translator(verticalVelocity * deltaTime, 0, 1, 0);

  auto newPos = leftT(position);
  if (!bbox.globalCollidesWith(newPos)) {
    position = newPos;
  }

  newPos = forwardT(position);
  if (!bbox.globalCollidesWith(newPos)) {
    position = newPos;
  }

  newPos = vertT(position);
  bbox.globalCollidesWith(position);
  if (bbox.globalCollidesWith(newPos)) {
    isGrounded = true;
    verticalVelocity = 0.f;
  } else {
    position = newPos;
    isGrounded = false;
  }

  auto swingT = line.restrictPosition(position, isGrounded, getPos());
  position = swingT(position);

  // // Apply combined transformations
  // const auto totalT = forwardT * leftT * vertT;
  // position = totalT(position);

  forwardT = kln::translator(); // Identity
  leftT = kln::translator();    // Identity

  // if (!isGrounded) {
  //   newPos = forwardT(position);
  //   if (!bbox.globalCollidesWith(newPos)) {
  //     position = newPos;
  //   }
  // }

  camera.updatePos(getPos());
  // line.updateStartPos(getPos());
}

void Player::drawLine(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix,
    UniformHandler handler) const
{
  line.draw(viewMatrix, projMatrix, handler);
}

void Player::createLine()
{
  isHooked = line.createLine({position.x(), position.y(), position.z()},
      camera.m_FrontVector, 20.0f, bbox);
}

void Player::clearLine()
{
  isHooked = false;
  line.clearVertex();
}

const glm::vec3 Player::getPos() const
{
  return glm::vec3(position.x(), position.y() + 0.5f, position.z());
}
