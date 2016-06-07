#include "ShaderProgram.h"

#include <iostream>

namespace VolViz {
namespace GL {

Shader::Shader(GLenum type, std::string const &source) {
  shader_ = glCreateShader(type);
  assertGL("Failed to create shader");
  auto const src = source.c_str();
  glShaderSource(shader_, 1, &src, nullptr);
  assertGL("Failed to upload shader source");
  compile();
}

void Shader::compile() const {
  glCompileShader(shader_);
  assertGL("Shader compilation failed");

  GLint success = 0;
  glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
  assertGL("Failed to get shader compile status");

  if (success) { return; }

  GLint logSize = 0;
  glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &logSize);
  assertGL("Failed to get shader info log length");
  std::vector<GLchar> log(static_cast<std::size_t>(logSize));

  glGetShaderInfoLog(shader_, logSize, &logSize, log.data());
  assertGL("Failed to get shader info log");

  std::cerr << "Failed to compile shader " << shader_ << ":" << std::endl
            << std::string(log.data()) << std::endl;
  throw std::runtime_error("Shader compilation failed");
}

ShaderProgram &ShaderProgram::link() {
  glLinkProgram(program_);
  assertGL("Failed to link program");

  // Check for linking errors
  GLint isLinked = GL_FALSE;
  glGetProgramiv(program_, GL_LINK_STATUS, &isLinked);

  if (!isLinked) {
    GLint maxLength = 0;
    glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> infoLog(static_cast<std::size_t>(maxLength));
    glGetProgramInfoLog(program_, maxLength, &maxLength, infoLog.data());

    throw std::runtime_error("Failed to link program: " +
                             std::string(infoLog.data()));
  }

  detachShaders();
  return *this;
}

} // namespace GL
} // namespace VolViz
