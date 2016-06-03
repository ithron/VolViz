#include "MeshViewer.hh"
#include "Mesh.hh"
#include "MeshParser.hh"

#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  using MeshViewer::Viewer;
  using Eigen::Vector3d;
  using MeshType = SBSSegmentation::Mesh<>;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " meshFile" << std::endl;
    return EXIT_FAILURE;
  }

  auto f = std::ifstream(argv[1]);
  std::cout << "Loading mesh " << argv[1] << "... " << std::flush;
  auto const mesh = SBSSegmentation::MeshParser::parseMesh<MeshType>(f);
  std::cout << "done." << std::endl;

  if (!mesh) {
    std::cerr << "Failed to load mesh " << argv[1] << std::endl;
    return EXIT_FAILURE;
  }

  auto viewer = Viewer{};

  viewer.setMesh(*mesh);

  viewer.start();

  while (viewer) { viewer.renderOneFrame(); }

  return EXIT_SUCCESS;
}
