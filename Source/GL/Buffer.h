#ifndef VolViz_Buffer_h
#define VolViz_Buffer_h

#include "GLdefs.h"
#include "Error.h"

#include <cstddef>

namespace VolViz {
namespace Private_ {
namespace GL {

/// RAII wrapper for OpenGL buffers
struct Buffer {
  /// Constructs an uninitialized buffer object, i.e. with no corresponding
  /// OpenGL buffer
  inline Buffer(int) noexcept {}

  inline Buffer() noexcept {
    glGenBuffers(1, &name);
    assertGL("Buffer creation failed");
  }

  inline ~Buffer() { glDeleteBuffers(1, &name); }

  Buffer(Buffer const &) = delete;
  inline Buffer(Buffer &&rhs) noexcept : name(rhs.name) { rhs.name = 0; }

  inline Buffer &operator=(Buffer &&rhs) noexcept {
    name = rhs.name;
    rhs.name = 0;
    return *this;
  }

  inline void bind(GLenum target) const noexcept {
    glBindBuffer(target, name);
    assertGL("Failed to bind buffer");
  }

  inline static void unbind(GLenum target) noexcept { glBindBuffer(target, 0); }

  template <class T>
  void upload(GLenum target, std::size_t size, T const *data,
              GLbitfield flags) noexcept {
    bind(target);
    glBufferData(target, static_cast<GLsizeiptr>(size),
                 static_cast<GLvoid const *>(data), flags);
  }

  GLuint name = 0;
};

} // namespace GL
} // namespace Private_
} // namespace VolViz

#endif // VolViz_Buffer_h
