#pragma once

#include "Geometry.h"

namespace VolViz {
namespace Private_ {

class VisualizerImpl;

class AxisAlignedPlane : public Geometry {
public:
  AxisAlignedPlane(AxisAlignedPlaneDescriptor const &descriptor,
                   VisualizerImpl &visualizer);

protected:
  virtual void doInit() override;

  virtual void doRender(std::uint32_t index, bool selected) override;
};

} // namespace Private_

} // namespace VolViz
