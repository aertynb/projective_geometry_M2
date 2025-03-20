#pragma once

#include "shaders.hpp"

class UniformHandler
{
public:
  UniformHandler(const GLProgram &program) : _program{program} { getUniform(); }

  GLuint uModelViewProjMatrix, uModelViewMatrix, uNormalMatrix;

private:
  void getUniform()
  {
    uModelViewProjMatrix =
        glGetUniformLocation(_program.glId(), "uModelViewProjMatrix");
    uModelViewMatrix =
        glGetUniformLocation(_program.glId(), "uModelViewMatrix");
    uNormalMatrix = glGetUniformLocation(_program.glId(), "uNormalMatrix");
  }

  const GLProgram &_program;
};