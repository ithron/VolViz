#include "MeshViewer.hh"

#include <Eigen/Core>
#include <igl/readOBJ.h>
#include <igl/readOFF.h>
#include <igl/readPLY.h>
#include <igl/readSTL.h>

#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  using MeshViewer::Viewer;
  using Eigen::Vector3d;
  using Vertices = Eigen::MatrixXd;
  using Triangles = Eigen::MatrixXi;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " meshFile" << std::endl;
    return EXIT_FAILURE;
  }

  auto f = std::ifstream(argv[1]);
  auto const filename = std::string(argv[1]);
  auto const ext = filename.substr(filename.size() - 3, 3);
  std::cout << "Loading mesh " << argv[1] << "... " << std::flush;
  Vertices V;
  Triangles T;
  if (ext == "off")
    igl::readOFF(filename, V, T);
  else if (ext == "ply")
    igl::readPLY(filename, V, T);
  else if (ext == "stl") {
    Vertices N;
    igl::readSTL(filename, V, T, N);
  } else if (ext == "obj")
    igl::readOBJ(filename, V, T);
  else {
    std::cerr << std::endl
              << "Unrecognized mesh format: " << ext << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "done." << std::endl;
  std::cout << V.rows() << " vertices, " << T.rows() << " triangles."
            << std::endl;

  Eigen::Vector3d const min = V.colwise().minCoeff();
  Eigen::Vector3d const max = V.colwise().maxCoeff();

  std::cout << "bbox: " << min.transpose() << " - " << max.transpose()
            << std::endl;

  auto viewer = Viewer{};

  viewer.setMesh((V * 100).eval(), T);

  viewer.start();

  while (viewer) { viewer.renderOneFrame(); }

  return EXIT_SUCCESS;
}