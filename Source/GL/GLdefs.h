// clang-format on
#ifndef XCODE_BUILD
#  include "config.h"
#endif
// clang-format off

#include "glad.h"
#include <GLFW/glfw3.h>

#ifndef GL_ZERO_TO_ONE
#define GL_ZERO_TO_ONE 0x935F
#endif

#if !defined(GL_ARB_clip_control) && !defined(GL_ARB_clip_control_PROTOTYPE)
#define GL_ARB_clip_control_PROTOTYPE
inline void glClipControl(GLenum, GLenum) {}
#endif
