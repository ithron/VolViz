// clang-format on
#ifndef XCODE_BUILD
#  include "config.h"
#endif
// clang-format off

#ifdef _WIN32
#define APIENTRY __stdcall
#endif
#include "glad.h"

#ifndef GL_ZERO_TO_ONE
#define GL_ZERO_TO_ONE 0x935F
#endif

