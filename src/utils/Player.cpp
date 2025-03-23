#include "Player.hpp"

void Player::moveUp(float _speed)
{
  const auto &frontVec = camera.m_FrontVector;
  if (!isHooked)
    forwardT =
        kln::translator(_speed * speed * deltaTime, frontVec.x, 0, frontVec.z);
  else {
    forwardT = kln::translator(
        _speed * speed * 2 * deltaTime, frontVec.x, frontVec.y, frontVec.z);
    verticalVelocity = 0.f;
  }
}

void Player::moveLeft(float _speed)
{
  const auto &leftVector = camera.m_LeftVector;
  leftT = kln::translator(
      _speed * speed * deltaTime, leftVector.x, 0, leftVector.z);
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
  if (!isGrounded) {
    verticalVelocity +=
        gravity * deltaTime; // Gravity decreases velocity over time
    verticalVelocity = std::clamp(verticalVelocity, -5.0f, 5.0f);
  }
}

void Player::update()
{
  applyGravity();

  // Apply jumping motion (always affects position)
  vertT = kln::translator(verticalVelocity * deltaTime, 0, 1, 0);

  // std::cout << verticalVelocity << std::endl;

  auto newPos = leftT(position);
  if (!bbox.globalCollidesWith(newPos)) {
    position = newPos;
  }

  newPos = forwardT(position);
  if (!bbox.globalCollidesWith(newPos)) {
    position = newPos;
  }

  auto swingT = line.restrictPosition(newPos, position, getPos());

  position = swingT(position);

  newPos = vertT(position);
  bbox.globalCollidesWith(position);
  if (bbox.globalCollidesWith(newPos)) {
    isGrounded = true;
    verticalVelocity = 0.f;
  } else {
    position = newPos;
    isGrounded = false;
  }

  forwardT = kln::translator(); // Identity
  leftT = kln::translator();    // Identity

  camera.updatePos(getPos());
}

void Player::drawLine(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix,
    UniformHandler handler) const
{
  line.draw(viewMatrix, projMatrix, handler);
}

void Player::createLine()
{
  isHooked = line.createLine({position.x(), position.y(), position.z()},
      camera.m_FrontVector, 10.0f, bbox);
}

void Player::clearLine()
{
  line.clearVertex();
  isHooked = false;
}

const glm::vec3 Player::getPos() const
{
  return glm::vec3(position.x(), position.y() + 0.5f, position.z());
}
