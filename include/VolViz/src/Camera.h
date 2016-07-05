#ifndef VolViz_Camera_h
#define VolViz_Camera_h

#include "AtomicCache.h"
#include "AtomicWrapper.h"
#include "Types.h"

namespace VolViz {

class VisualizerImpl;
class CameraClient;

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
  CameraClient client() const noexcept;

private:
  friend class CameraClient;

  /// Returns the camera's projection matrix
  Matrix4 projectionMatrix() const noexcept;

  /// Returns the camera's view matrix, i.e. the inverse camera transform
  Matrix4 viewMatrix() const noexcept;

  /// Returns the product of projectionMatrix() * viewMatrix(...)
  Matrix4 viewProjectionMatrix() const noexcept;

  /// Cached projection matrix
  mutable AtomicCache<Matrix4> cachedProjectionMatrix_{
      [this]() { return projectionMatrix(); }};

  /// Cached view Matrix
  mutable AtomicCache<Matrix4> cachedViewMatrix_{
      [this]() { return viewMatrix(); }};

  /// Cached product viewMatrix * projectionMatrix
  mutable AtomicCache<Matrix4> cachedViewProjectionMatrix_{
      [this]() { return viewProjectionMatrix(); }};

  /// Cached scale of last call to viewMatrix(...)
  mutable Length cachedScale_;
};

class CameraClient {
  friend class VisualizerImpl;
  friend class Camera;

  CameraClient(Camera const &cam) : cam_(cam) {}

  /// Returns the camera's projection matrix
  inline Matrix4 projectionMatrix() const noexcept {
    return cam_.cachedProjectionMatrix_;
  }

  /// Returns the camera's view matrix, i.e. the inverse camera transform
  inline Matrix4 viewMatrix(Length ambientScale) const noexcept {
    using std::abs;
    using namespace literals;

    if (abs(ambientScale - cam_.cachedScale_) < 1e-12_nm) {
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

    if (abs(ambientScale - cam_.cachedScale_) < 1e-12_nm) {
      cam_.cachedViewMatrix_.markAsDirty();
      cam_.cachedViewProjectionMatrix_.markAsDirty();
      cam_.cachedScale_ = ambientScale;
    }
    return cam_.cachedViewProjectionMatrix_;
  }

  Camera const &cam_;
};

} // namespace VolViz

#endif // VolViz_Camera_h
