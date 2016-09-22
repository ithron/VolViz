#include <VolViz/VolViz.h>

#include <Eigen/Core>
#include <igl/readOBJ.h>
#include <igl/readOFF.h>
#include <igl/readPLY.h>
#include <igl/readSTL.h>

#include <fstream>
#include <iostream>

auto generateVolume();

auto generateVolume() {
  using namespace VolViz;
  using namespace VolViz::literals;
  Size3 const size(256, 256, 128);
  auto const nVoxels = size(0) * size(1) * size(2);

  std::vector<Color> data;
  data.reserve(3 * nVoxels);
  Color c = Colors::Black();
  for (unsigned int z = 0; z < size(2); ++z) {
    c(2) = (static_cast<float>(z) / static_cast<float>(size(2) - 1));
    for (unsigned int y = 0; y < size(1); ++y) {
      c(1) = (static_cast<float>(y) / static_cast<float>(size(1) - 1));
      for (unsigned int x = 0; x < size(0); ++x) {
        c(0) = (static_cast<float>(x) / static_cast<float>(size(0) - 1));
        data.push_back(c);
      }
    }
  }

  VolumeDescriptor v;
  v.size = size;
  v.voxelSize = {{100_um, 100_um, 200_um}};
  v.type = VolumeType::ColorRGB;

  return std::make_pair(v, data);
}

int main(int argc, char **argv) {
  using namespace VolViz;
  using namespace VolViz::literals;
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
    std::cerr << std::endl << "Unrecognized mesh format: " << ext << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "done." << std::endl;
  std::cout << V.rows() << " vertices, " << T.rows() << " triangles."
            << std::endl;

  Eigen::Vector3d const min = V.colwise().minCoeff();
  Eigen::Vector3d const max = V.colwise().maxCoeff();

  Length meshScale = 5_cm;

  std::cout << "bbox: " << min.transpose() << " - " << max.transpose()
            << std::endl;
  std::cout << "bbox size: "
            << ((max - min).transpose() * static_cast<double>(meshScale / 1_mm))
            << " mm" << std::endl;

  //  Eigen::MatrixXd V(3, 3);
  //  V << 0, 1, 0, -1, -1, 0, 1, -1, 0;
  //  Eigen::MatrixXi T(1, 3);
  //  T << 0, 1, 2;

  auto viewer = Visualizer{};

  // Add mesh
  MeshDescriptor mesh;
  mesh.vertices = V.cast<float>();
  mesh.indices = T.cast<std::uint32_t>();
  mesh.movable = true;
  mesh.scale = 50_mm;
  mesh.color = Colors::White();
  viewer.addGeometry("Mesh", mesh);

  viewer.start();
  viewer.showGrid = true;

  Light light;
  light.ambientFactor = 1.0f;
  light.color = Colors::White();
  light.position = PositionH(1, 1, 1, 0);

  viewer.addLight(0, light);

  light.position = PositionH(2, 1, 1, 0);
  viewer.addLight(1, light);

  light.position = PositionH(1, 2, 1, 0);
  viewer.addLight(2, light);

  viewer.scale = 1_mm;

  AxisAlignedPlaneDescriptor plane;
  plane.axis = Axis::X;
  // plane.color = Colors::Green();
  plane.color = Colors::White();
  plane.intercept = 0_mm;

  viewer.addGeometry("X-Plane", plane);

  plane.axis = Axis::Y;
  // plane.color = Colors::Blue();
  plane.color = Colors::White();
  plane.intercept = 0_mm;
  viewer.addGeometry("Y-Plane", plane);

  plane.axis = Axis::Z;
  // plane.color = Colors::Red();
  plane.color = Colors::White();
  plane.intercept = 0_mm;
  viewer.addGeometry("Z-Plane", plane);

  std::cout << "Generating volume... " << std::flush;
  auto const vol = generateVolume();
  std::cout << "done." << std::endl;

  viewer.setVolume(vol.first, gsl::as_span(vol.second));

  while (viewer) { viewer.renderOneFrame(); }

  return EXIT_SUCCESS;
}
