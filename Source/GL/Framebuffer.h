#ifndef VolViz_Framebuffer_h
#define VolViz_Framebuffer_h

#include "GLdefs.h"

namespace VolViz {
namespace Private_ {
namespace GL {

/// RAII wrapper for OpenGL framebuffer objects
struct Framebuffer {

  /// Creates a uninitialized frabebuffer object, i.e. a fbo that corresponds
  /// with the default FBO
  inline Framebuffer(int) noexcept {}

  inline Framebuffer() noexcept { glGenFramebuffers(1, &name); }

  inline ~Framebuffer() { glDeleteFramebuffers(1, &name); }

  inline Framebuffer(Framebuffer &&rhs) noexcept {
    using std::swap;
    swap(name, rhs.name);
  }

  inline Framebuffer &operator=(Framebuffer &&rhs) noexcept {
    using std::swap;
    swap(name, rhs.name);
    return *this;
  }

  inline void bind(GLenum target) const noexcept {
    glBindFramebuffer(target, name);
    assertGL("Failed to bind FBO");
  }

  inline static void unbind(GLenum target) noexcept {
    glBindFramebuffer(target, 0);
  }

  GLuint name = 0;
};

} // namespace GL
} // namespace Private_
} // namespace VolViz

#endif // VolViz_Framebuffer_h
