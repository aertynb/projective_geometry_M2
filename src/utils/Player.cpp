#include "Player.hpp"

void Player::moveUp(float speed)
{
  const auto &frontVec = camera.m_FrontVector;
  forwardT = kln::translator(speed * deltaTime, frontVec.x, 0, frontVec.z);
}

void Player::moveLeft(float speed)
{
  const auto &leftVector = camera.m_LeftVector;
  leftT = kln::translator(speed * deltaTime, leftVector.x, 0, leftVector.z);
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
  }
}

void Player::update()
{
  applyGravity();

  // Apply jumping motion (always affects position)
  vertT = kln::translator(verticalVelocity * deltaTime, 0, 1, 0);

  auto newPos = vertT(position);
  bbox.globalCollidesWith(position);
  if (bbox.globalCollidesWith(newPos)) {
    isGrounded = true;
    verticalVelocity = 0.f;
    std::cout << "collision vert" << std::endl;
  } else {
    position = newPos;
    isGrounded = false;
  }

  newPos = leftT(position);
  if (!bbox.globalCollidesWith(newPos)) {
    position = newPos;
  }

  newPos = forwardT(position);
  if (!bbox.globalCollidesWith(newPos)) {
    position = newPos;
  }

  // // Apply combined transformations
  // const auto totalT = forwardT * leftT * vertT;
  // position = totalT(position);

  // Prevent movement reset (use identity transformation)
  forwardT = kln::translator(); // Identity
  leftT = kln::translator();    // Identity

  camera.updatePos(getPos());
}

const glm::vec3 Player::getPos() const
{
  return glm::vec3(position.x(), position.y() + 0.5f, position.z());
}
