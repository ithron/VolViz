#ifndef VolViz_Types_h
#define VolViz_Types_h

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <gsl.h>
#include <phys/units/quantity.hpp>

namespace VolViz {

using Position = Eigen::Vector3f;
using PositionH = Eigen::Vector4f;

using Orientation = Eigen::Quaternionf;

using Scale = float;

using Length = phys::units::quantity<phys::units::length_d>;

using phys::units::meter;

using phys::units::centi;
using phys::units::milli;
using phys::units::micro;
using phys::units::nano;

namespace literals = phys::units::literals;

} // namespace VolViz

#endif // VolViz_Types_h
