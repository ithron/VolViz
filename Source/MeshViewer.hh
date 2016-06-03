#ifndef SBSSegmentation_MeshViewer_hh
#define SBSSegmentation_MeshViewer_hh

#include "Mesh.hh"

#include <memory>

namespace MeshViewer {

class MeshViewerImpl;

class Viewer {
public:
  static auto constexpr kDefaultWidth = 1024;
  static auto constexpr kDefaultHeight = 768;
  static auto constexpr kDefaultFOV = 170;
  static auto constexpr kTitle = "Mesh Viewer";

  template <class... Attrs> using Mesh = SBSSegmentation::Mesh<Attrs...>;

  Viewer();

  ~Viewer();

  Viewer(Viewer const &) = delete;
  Viewer(Viewer &&);

  Viewer &operator=(Viewer const &) = delete;
  Viewer &operator=(Viewer &&);

  void start();

  void renderOneFrame();

  operator bool() const noexcept;

  template <class... Attributes> void setMesh(Mesh<Attributes...> const &mesh);

private:
  std::unique_ptr<MeshViewerImpl> impl_;
};

} // namespace MeshViewer

#endif // SBSSegmentation_MeshViewer_hh
