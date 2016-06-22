#ifndef VolViz_Types_h
#define VolViz_Types_h

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <gsl.h>
#include <phys/units/quantity.hpp>

namespace VolViz {

/// Position in 3D euclidean space
using Position = Eigen::Vector3f;
/// Position in homogenous coordinates
using PositionH = Eigen::Vector4f;

/// Normalized RGB color
using Color = Eigen::Vector3f;

/// 6-DOF orientation, represented as a quaternion
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
