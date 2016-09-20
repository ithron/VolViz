#pragma once

#include "Geometry.h"

namespace VolViz {

namespace Private_ {

class AxisAlignedPlane : public Geometry {
protected:
  virtual void doInit() override;

  virtual void doRender() override;

private:
  friend class GeometryFactory;

  AxisAlignedPlane(AxisAlignedPlaneDescriptor const &descriptor);

  AxisAlignedPlaneDescriptor descriptor_;
};

} // namespace Private_

} // namespace VolViz
