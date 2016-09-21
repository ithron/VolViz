//
//  Error.h
//  VolViz
//
//  Created by Stefan Reinhold on 07.06.16.
//
//

#ifndef VolViz_Error_h
#define VolViz_Error_h

#include "GLdefs.h"

#include <cassert>

#ifndef NDEBUG
inline void assertGL(char const *txt) noexcept {
  auto const err = glGetError();
  switch (err) {
  case GL_NO_ERROR:
    break;
  case GL_INVALID_ENUM:
    assert(false && "Error INVALID_ENUM:" && txt);
  case GL_INVALID_VALUE:
    assert(false && "Error GL_INVALID_VALUE:" && txt);
  case GL_INVALID_OPERATION:
    assert(false && "Error GL_INVALID_OPERATION:" && txt);
  case GL_OUT_OF_MEMORY:
    assert(false && "Error GL_OUT_OF_MEMORY:" && txt);
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    assert(false && "Error GL_INVALID_FRAMEBUFFER_OPERATION:" && txt);
  default:
    assert(false && "Unknown error: " && txt);
  }
}
#else
#define assertGL(txt)
#endif

#endif // VolViz_Error_h
