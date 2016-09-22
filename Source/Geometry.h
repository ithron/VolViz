#pragma once

#include "GeometryDescriptor.h"

namespace VolViz {
namespace Private_ {

class VisualizerImpl;

class Geometry {
public:
  using UniquePtr = std::unique_ptr<Geometry>;

  Position position{Position::Zero()};
  Orientation orientation{Orientation::Identity()};
  Length scale{1 * milli * meter};
  MoveMask moveMask{MoveMask::All};
  Color color{Colors::White()};

  virtual ~Geometry() = default;

  void init();

  void render(std::uint32_t index, bool selected);

protected:
  Geometry(VisualizerImpl &visualizer);
  Geometry(GeometryDescriptor const &descriptor, VisualizerImpl &visualizer);

  virtual void doInit();

  virtual void doRender(std::uint32_t index, bool selected) = 0;

  VisualizerImpl &visualizer_;
};

} // namespace VolViz
} // namespace Private_
