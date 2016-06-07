#ifndef VolViz_ShaderProgram_h
#define VolViz_ShaderProgram_h

#include "Error.h"
#include "GL.h"

#include <vector>

namespace VolViz {
namespace GL {

class ShaderProgram;

/// RAII wrapper for OpenGL shader objects
class Shader {
public:
  Shader(GLenum type, std::string const &source);

  inline ~Shader() { glDeleteShader(shader_); }

  Shader(Shader const &) = delete;

  inline Shader(Shader &&rhs) noexcept {
    using std::swap;
    swap(shader_, rhs.shader_);
  }

  inline Shader &operator=(Shader &&rhs) noexcept {
    using std::swap;
    swap(shader_, rhs.shader_);
    return *this;
  }

private:
  void compile() const;

  friend class ShaderProgram;

  GLuint shader_;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/// RAII wrapper for OpenGL shader programs objects
class ShaderProgram {
public:
  inline ShaderProgram() noexcept {
    program_ = glCreateProgram();
    assertGL("Shader program creationf ailed");
  }

  inline ~ShaderProgram() {
    detachShaders();
    glDeleteProgram(program_);
  }

  ShaderProgram(ShaderProgram const &) = delete;

  inline ShaderProgram(ShaderProgram &&rhs) noexcept {
    using std::swap;
    swap(program_, rhs.program_);
    swap(attachedShaders_, rhs.attachedShaders_);
  }

  inline ShaderProgram &operator=(ShaderProgram &&rhs) noexcept {
    using std::swap;
    swap(program_, rhs.program_);
    swap(attachedShaders_, rhs.attachedShaders_);
    return *this;
  }

  inline ShaderProgram &attachShader(Shader const &shader) noexcept {
    glAttachShader(program_, shader.shader_);
    assertGL("Shader attachment failed");
    attachedShaders_.push_back(shader.shader_);
    return *this;
  }

  template <class Container>
  inline ShaderProgram &attachShaders(Container &&c) noexcept {
    for (auto const &s : std::forward<Container>(c)) attachShader(s);
  }

  ShaderProgram &link();

  inline void use() const noexcept {
    assertGL("Dirty OpenGL error stack");
    glUseProgram(program_);
    assertGL("Failed to use program");
  }

// private:
  inline void detachShaders() noexcept {
    for (auto s : attachedShaders_) glDetachShader(program_, s);
    attachedShaders_.clear();
  }

  GLuint program_;
  std::vector<GLuint> attachedShaders_;
};
#pragma clang diagnostic pop

} // namespace GL
} // namespace VolViz

#endif // VolViz_ShaderProgram_h
