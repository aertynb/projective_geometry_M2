#pragma once

#include "glad/glad.h"
#include "uniformHandler.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <vector>

// Represents a single vertex of a line
struct LineVertex
{
  LineVertex(const glm::vec3 &pos) : position{pos} {}

  static unsigned long sizeOfVertex() { return sizeof(position); }

  glm::vec3 position; // Vertex position
};

class LineCustom
{
public:
  LineCustom() {}

  const LineVertex *getDataPointer() const { return &m_Vertices[0]; }
  GLsizei getVertexCount() const { return m_nVertexCount; }
  const unsigned long getVertexSize() const
  {
    return LineVertex::sizeOfVertex();
  }

  void clearVertex()
  {
    m_Vertices.clear();
    m_nVertexCount = 0;
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    drawing = false;
  }

  void createLine(
      const glm::vec3 &start, const glm::vec3 &m_FrontVector, float maxDist)
  {
    const auto center = glm::vec3(start.x, start.y - 0.5f, start.z);
    glm::vec3 end =
        center + (glm::normalize(m_FrontVector) * toLineOfSight(maxDist, 0.5f));
    end.y += 0.5f;
    // std::cout << center << ", " << end << std::endl;
    build(center, end);
    initObj(0);
    drawing = true;
  }

  const float toLineOfSight(float maxDist, float center) const
  {
    return sqrt(pow(maxDist, 2) + pow(center, 2));
  }

  void initObj(GLuint vPos)
  {
    initVboPointer();
    initVaoPointer(vPos);
  }

  void draw(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix,
      const UniformHandler &handler) const
  {
    if (drawing) {
      //   std::cout << "drawing" << std::endl;
      const auto mvMatrix = viewMatrix * glm::mat4(1.f);
      const auto mvpMatrix = projMatrix * mvMatrix;
      const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));
      glUniformMatrix4fv(
          handler.uModelViewProjMatrix, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
      glUniformMatrix4fv(
          handler.uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(mvMatrix));
      glUniformMatrix4fv(
          handler.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBindVertexArray(vao);
      glDrawArrays(GL_LINES, 0, getVertexCount());
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }

private:
  void build(const glm::vec3 &start, const glm::vec3 &end)
  {
    LineVertex vertices[] = {start, end};

    m_Vertices.assign(vertices, vertices + 2);
    m_nVertexCount = 2;
  }

  void initVaoPointer(GLuint vPos)
  {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(vPos);
    glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, getVertexSize(),
        (GLvoid *)offsetof(LineVertex, position));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void initVboPointer()
  {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, getVertexCount() * getVertexSize(),
        getDataPointer(), GL_STATIC_DRAW);
  }

  std::vector<LineVertex> m_Vertices;
  GLsizei m_nVertexCount;
  GLuint vbo;
  GLuint vao;
  bool drawing;
};
