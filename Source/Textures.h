#ifndef VolViz_Texture_h
#define VolViz_Texture_h

#include "Error.h"
#include "GL.h"

#include <array>

namespace VolViz {
namespace GL {

/// RAII wrapper for arrays of OpenGL texture objects
template <std::size_t N> struct Textures {

  inline Textures(int) noexcept {
    for (std::size_t i = 0; i < N; ++i) names[i] = 0;
  }

  inline Textures() noexcept { glGenTextures(N, names.data()); }

  inline Textures(Textures &&rhs) noexcept {
    using std::swap;
    swap(names, rhs.names);
  }

  inline ~Textures() { glDeleteTextures(N, names.data()); }

  inline Textures &operator=(Textures &&rhs) noexcept {
    using std::swap;
    swap(names, rhs.names);
    return *this;
  }

  std::array<GLuint, N> names;
};

} // namespace GL
} // namespace VolViz

#endif // VolViz_Texture_h
