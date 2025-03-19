#include "Player.hpp"

void Player::moveUp(float speed)
{
  const auto &direction = camera.m_FrontVector;
  kln::translator r{speed, direction.x, 0.f, direction.z};
  position = r(position);
}

void Player::moveLeft(float speed)
{
  const auto &direction = camera.m_LeftVector;
  kln::translator r{speed, direction.x, direction.y, direction.z};
  position = r(position);
}

void Player::update() { camera.updatePos(getPos()); }

const glm::vec3 Player::getPos() const
{
  return glm::vec3(position.x(), position.y(), position.z());
}