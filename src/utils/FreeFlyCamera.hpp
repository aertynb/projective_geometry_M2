#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace glm
{
struct FreeflyCamera
{
public:
  glm::vec3 m_FrontVector, m_LeftVector;

  FreeflyCamera(const glm::vec3 &pos) :
      m_fPhi{M_PI}, m_fTheta{0.}, m_Position{pos}
  {
    computeDirectionVectors();
  }

  void tryMoveLeft(float t) { m_Position = m_Position + t * m_LeftVector; }

  void tryMoveUp(float t) { m_Position = m_Position + t * m_FrontVector; }

  void rotateLeft(float degrees)
  {
    m_fPhi -= glm::radians(degrees) * 5;
    computeDirectionVectors();
  }

  void rotateUp(float degrees)
  {
    m_fTheta += glm::radians(degrees) * 5;
    if (m_fTheta > 89.0f)
      m_fTheta = 89.0f;
    if (m_fTheta < -89.0f)
      m_fTheta = -89.0f;
    computeDirectionVectors();
  }

  glm::mat4 getViewMatrix() const
  {
    return lookAt(
        m_Position, m_Position + m_FrontVector, glm::normalize(m_UpVector));
  }

  const glm::vec3 getPosition() const { return m_Position; }

  void updatePos(const glm::vec3 &pos) { m_Position = pos; }

private:
  glm::vec3 m_Position, m_UpVector;
  float m_fPhi, m_fTheta;

  void computeDirectionVectors()
  {
    auto radTheta = glm::radians(m_fTheta);
    auto radPhi = glm::radians(m_fPhi);
    m_FrontVector = glm::vec3(cos(radTheta) * sin(radPhi), sin(radTheta),
        cos(radTheta) * cos(radPhi));
    m_LeftVector = glm::vec3(sin(radPhi + M_PI / 2), 0, cos(radPhi + M_PI / 2));
    m_UpVector = glm::cross(m_FrontVector, m_LeftVector);
  }
};
} // namespace glm