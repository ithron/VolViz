#pragma once

#include "GeometryDescriptor.h"

namespace VolViz {
namespace Private_ {

class Geometry {
public:
  using UniquePtr = std::unique_ptr<Geometry>;

  Position position{Position::Zero()};
  Orientation orientation{Orientation::Identity()};
  Length scale{1_mm};
  MoveMask moveMask{MoveMask::All};
  Color color{Color::White()};

  virtual ~Geometry() = default;

  void init();

  void render(std::uint32_t index, bool selected);

protected:
  Geometry() = default;

  virtual void doInit();

  virtual void doRender(std::uint32_t index, bool selected) = 0;
};

} // namespace VolViz
} // namespace Private_
