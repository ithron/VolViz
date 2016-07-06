#ifndef VolViz_Camera_h
#define VolViz_Camera_h

#include "AtomicCache.h"
#include "AtomicWrapper.h"
#include "Types.h"

namespace VolViz {

namespace Private_ {
class CameraClient;
} // namespace Private_

/// Basic camera class.
/// The camera has a physical location, an orientation and a field of view.
/// The field of view is specified as the horizontal FOV, the vertical FOV is
/// computed from the apect radius.
///
/// @note All properties are thread safe.
class Camera {
  template <class T> using Property = AtomicWrapper<T, SetAndNotifyPolicy>;

public:
  Camera() noexcept {
    orientation.afterAction = [this](auto const &) {
      cachedViewMatrix_.markAsDirty();
      cachedViewProjectionMatrix_.markAsDirty();
    };

    position.afterAction = [this](auto const &) {
      cachedViewMatrix_.markAsDirty();
      cachedViewProjectionMatrix_.markAsDirty();
    };

    verticalFieldOfView.afterAction = [this](auto const &) {
      cachedVerticalFOV_.markAsDirty();
      cachedProjectionMatrix_.markAsDirty();
      cachedViewProjectionMatrix_.markAsDirty();
    };

    aspectRatio.afterAction = [this](auto const &) {
      cachedProjectionMatrix_.markAsDirty();
      cachedViewProjectionMatrix_.markAsDirty();
    };
  }

  /// The camera's orientation
  Property<Orientation> orientation{Orientation::Identity()};

  /// Position of the camera plane in space, i.e. the orientation of the
  /// camera projection plane's normal
  Property<PhysicalPosition> position{
      PhysicalPosition{0 * milli * meter, 0 * milli *meter, 0 * milli *meter}};

  /// Vertical field of view in rad
  Property<Angle> verticalFieldOfView{110 * degree_angle};

  /// Aspect ratio (width / height) or horizontal FOV / vertical FOV
  Property<float> aspectRatio{4.f / 3.f};

  /// Returns a client object that can be used to access cached projection and
  /// view matrices, as well as methods like unprojecting.
  /// @see CameraClient
  /// @note The retunred CameraClient object can only be accesses by
  /// an VisualizerImpl instance to prevent race coditions.
  Private_::CameraClient client() const noexcept;

private:
  friend class Private_::CameraClient;

  /// Returns the camera's projection matrix
  Matrix4 projectionMatrix() const noexcept;

  /// Returns the camera's view matrix, i.e. the inverse camera transform
  Matrix4 viewMatrix() const noexcept;

  /// Returns the product of projectionMatrix() * viewMatrix(...)
  Matrix4 viewProjectionMatrix() const noexcept;

  /// Unprojectis a point in screen coordinates with known deoth into the 3D
  /// scene
  ///
  /// @param screenPos the source point in screen coordinate system, i.e.
  /// (-1, -1) is the bottom left, (1, 1) is the top right
  ///
  /// @param depth the depth of the pixel at screenPos
  /// @param ambientLength the physical length of one unit in the target 3D
  /// space
  Position unproject(Position2 const &screenPos, float depth,
                     Length ambientScale) const noexcept;

  /// Cached projection matrix
  mutable AtomicCache<Matrix4> cachedProjectionMatrix_{
      [this]() { return projectionMatrix(); }};

  /// Cached view Matrix
  mutable AtomicCache<Matrix4> cachedViewMatrix_{
      [this]() { return viewMatrix(); }};

  /// Cached product viewMatrix * projectionMatrix
  mutable AtomicCache<Matrix4> cachedViewProjectionMatrix_{
      [this]() { return viewProjectionMatrix(); }};

  /// Cached vertical FOV
  mutable AtomicCache<Angle> cachedVerticalFOV_{
      [this]() { return static_cast<Angle>(verticalFieldOfView); }};

  /// Cached scale of last call to viewMatrix(...)
  mutable Length cachedScale_;
};

namespace Private_ {
class VisualizerImpl;

class CameraClient {
  friend class VisualizerImpl;
  friend class ::VolViz::Camera;

  CameraClient(Camera const &cam) : cam_(cam) {}

  /// Returns the camera's projection matrix
  inline Matrix4 projectionMatrix() const noexcept {
    return cam_.cachedProjectionMatrix_;
  }

  /// Returns the camera's view matrix, i.e. the inverse camera transform
  inline Matrix4 viewMatrix(Length ambientScale) const noexcept {
    using std::abs;
    using namespace literals;

    Expects(ambientScale > 0_mm);

    if (abs(ambientScale - cam_.cachedScale_) > 1e-3_nm) {
      cam_.cachedViewMatrix_.markAsDirty();
      cam_.cachedViewProjectionMatrix_.markAsDirty();
      cam_.cachedScale_ = ambientScale;
    }
    return cam_.cachedViewMatrix_;
  }

  /// Returns the product of projectionMatrix() * viewMatrix(...)
  inline Matrix4 viewProjectionMatrix(Length ambientScale) const noexcept {
    using std::abs;
    using namespace literals;

    Expects(ambientScale > 0_mm);

    if (abs(ambientScale - cam_.cachedScale_) > 1e-3_nm) {
      cam_.cachedViewMatrix_.markAsDirty();
      cam_.cachedViewProjectionMatrix_.markAsDirty();
      cam_.cachedScale_ = ambientScale;
    }
    return cam_.cachedViewProjectionMatrix_;
  }

  /// Unprojectis a point in screen coordinates with known deoth into the 3D
  /// scene
  ///
  /// @param screenPos the source point in screen coordinate system, i.e.
  /// (-1, -1) is the bottom left, (1, 1) is the top right
  ///
  /// @param depth the depth of the pixel at screenPos
  /// @param ambientLength the physical length of one unit in the target 3D
  /// space
  inline Position unproject(Position2 const &screenPos, float depth,
                            Length ambientScale) const noexcept {
    return cam_.unproject(screenPos, depth, ambientScale);
  }

  Camera const &cam_;
};

} // namespace Private_

} // namespace VolViz

#endif // VolViz_Camera_h
