#pragma once

#include "Geometry.h"

namespace VolViz {

class Visualizer;

namespace Private_ {

class AxisAlignedPlane : public Geometry {
protected:
  virtual void doInit() override;

  virtual void doRender() override;

private:
  friend class GeometryFactory;

  AxisAlignedPlane(AxisAlignedPlaneDescriptor const &descriptor,
                   Visualizer &visualizer);

  Visualizer &visualizer_;
};

} // namespace Private_

} // namespace VolViz
