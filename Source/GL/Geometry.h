#ifndef VolViz_GL_Geometry_h
#define VolViz_GL_Geometry_h

#include "Types.h"

namespace VolViz {
namespace Private_ {
namespace GL {

enum MoveMask : uint8_t {
  None = 0x00,
  X = 0x01,
  Y = 0x02,
  Z = 0x04,
  All = 0x07
};

struct Geometry {
  Position position{Position::Zero()};
  Orientation orientation{Orientation::Identity()};
  bool movable{true};
  MoveMask moveMask{All};
  Length scale {1 * milli * meter};
  Color color{Colors::White()};
};

} // namespace GL
} // namespace Private_
} // namespace VolViz

#endif // VolViz_GL_Geometry_h
