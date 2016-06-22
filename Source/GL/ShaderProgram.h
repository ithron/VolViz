#ifndef VolViz_ShaderProgram_h
#define VolViz_ShaderProgram_h

#include "GLdefs.h"
#include "Error.h"

#include <Eigen/Core>

#include <string>
#include <unordered_map>
#include <vector>

namespace VolViz {
namespace Private_ {
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

class UniformProxy {
public:
  inline UniformProxy(GLint loccation) noexcept : location_(loccation) {}

  UniformProxy(UniformProxy const &) = delete;
  UniformProxy(UniformProxy &&) = default;

  UniformProxy const &operator=(float f) const noexcept {
    assertGL("Precondition violation");
    glUniform1f(location_, f);
    assertGL("Failed to upload uniform");
    return *this;
  }

  UniformProxy const &operator=(GLint i) const noexcept {
    assertGL("Precondition violation");
    glUniform1i(location_, i);
    assertGL("Failed to upload uniform");
    return *this;
  }

  UniformProxy const &operator=(Eigen::Vector2f const &v) const noexcept {
    assertGL("Precondition violation");
    glUniform2fv(location_, 1, v.data());
    assertGL("Failed to upload uniform");
    return *this;
  }

  UniformProxy const &operator=(Eigen::Vector3f const &v) const noexcept {
    assertGL("Precondition violation");
    glUniform3fv(location_, 1, v.data());
    assertGL("Failed to upload uniform");
    return *this;
  }

  UniformProxy const &operator=(Eigen::Vector4f const &v) const noexcept {
    assertGL("Precondition violation");
    glUniform4fv(location_, 1, v.data());
    assertGL("Failed to upload uniform");
    return *this;
  }

  UniformProxy const &operator=(Eigen::Matrix4f const &m) const noexcept {
    assertGL("Precondition violation");
    glUniformMatrix4fv(location_, 1, false, m.data());
    assertGL("Failed to upload uniform");
    return *this;
  }

  UniformProxy const &
  operator=(Eigen::Transpose<Eigen::Matrix4f> const &m) const noexcept {
    assertGL("Precondition violation");
    glUniformMatrix4fv(location_, 1, true, m.nestedExpression().data());
    assertGL("Failed to upload uniform");
    return *this;
  }

  UniformProxy const &operator=(Eigen::Matrix3f const &m) const noexcept {
    assertGL("Precondition violation");
    glUniformMatrix3fv(location_, 1, true, m.data());
    assertGL("Failed to upload uniform");
    return *this;
  }

private:
  GLint const location_;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/// RAII wrapper for OpenGL shader programs objects
class ShaderProgram {
public:
  inline ShaderProgram() noexcept {
    assertGL("OpenGL error stack not clean");
    program_ = glCreateProgram();
    assert(program_ != 0 && "Shader program creation failed");
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
    swap(uniforms_, rhs.uniforms_);
  }

  inline ShaderProgram &operator=(ShaderProgram &&rhs) noexcept {
    using std::swap;
    swap(program_, rhs.program_);
    swap(attachedShaders_, rhs.attachedShaders_);
    swap(uniforms_, rhs.uniforms_);
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
    return *this;
  }

  ShaderProgram &link();

  inline void use() const noexcept {
    assertGL("Dirty OpenGL error stack");
    glUseProgram(program_);
    assertGL("Failed to use program");
  }

  inline UniformProxy const &operator[](std::string const &name) const {
    auto search = uniforms_.find(name);
    if (search != uniforms_.end()) return search->second;

    throw std::runtime_error(name +
                             " is not an active uniform of shader program " +
                             std::to_string(program_));
    // Dummy return statement because GCC is complaining, this code is never
    // reached!
    return uniforms_.begin()->second;
  }

  // private:
  inline void detachShaders() noexcept {
    for (auto s : attachedShaders_) glDetachShader(program_, s);
    attachedShaders_.clear();
  }

  void queryUniforms();

  using UniformTable = std::unordered_map<std::string, UniformProxy>;

  std::vector<GLuint> attachedShaders_;
  UniformTable uniforms_;
  GLuint program_ = 0;
};
#pragma clang diagnostic pop

} // namespace GL
} // namespace Private_
} // namespace VolViz

#endif // VolViz_ShaderProgram_h
