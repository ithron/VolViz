#pragma once

#include "Geometry.h"

namespace VolViz {
namespace Private_ {

class VisualizerImpl;

class AxisAlignedPlane : public Geometry {
protected:
  virtual void doInit() override;

  virtual void doRender(std::uint32_t index, bool selected) override;

private:
  friend class GeometryFactory;

  AxisAlignedPlane(AxisAlignedPlaneDescriptor const &descriptor,
                   VisualizerImpl &visualizer);

  Visualizer &visualizer_;
};

} // namespace Private_

} // namespace VolViz
