#ifndef VolViz_VertexArray_h
#define VolViz_VertexArray_h

#include "GLdefs.h"

namespace VolViz {
namespace Private_ {
namespace GL {

/// RAII wrapper for OpenGL vertex arrays
struct VertexArray {
  /// Constructs an uninitialized vertex array, i.e. with no corresponing
  /// OpenGL vertex array
  inline VertexArray(int) noexcept {}

  inline VertexArray() noexcept {
    glGenVertexArrays(1, &name);
    assertGL("Vertex array creation failed");
  }

  inline ~VertexArray() { glDeleteVertexArrays(1, &name); }

  inline VertexArray(VertexArray &&rhs) noexcept : name(rhs.name) {
    rhs.name = 0;
  }

  inline VertexArray &operator=(VertexArray &&rhs) noexcept {
    name = rhs.name;
    rhs.name = 0;
    return *this;
  }

  inline void bind() const noexcept {
    glBindVertexArray(name);
    assertGL("Failed to bind vertex array");
  }

  inline static void unbind() noexcept { glBindVertexArray(0); }

  inline VertexArray &enableVertexAttribArray(GLuint idx) noexcept {
    bind();
    glEnableVertexAttribArray(idx);
    assertGL("Failed to enabkle vertex attribute");
    return *this;
  }

  GLuint name = 0;
};

} // namespace GL
} // namespace Private_
} // namespace VolViz

#endif // VolViz_VertexArray_h
