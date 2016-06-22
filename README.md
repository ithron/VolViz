# VolViz [![Build Status](https://travis-ci.org/ithron/VolViz.svg?branch=feature%2Flighting)](https://travis-ci.org/ithron/VolViz)
Volumetric image visualization tool, e.g. for CT images

# Build Instruction
```shell
git clone git@github.com:ithron/VolViz.git
cd VolViz
git submodule init
git submodule update
mkdir ../VolViz-build
cd ../VolViz-build
cmake ../VolViz
make
```
# Usage
## CMake
Set `VolViz_DIR` to the build directory,
in your CMakeLists.txt add `find_package(VolViz)`. And add the `VolViz::Visualizer` to your dependencies, e.g.:
```cmake
target_link_libraries(MyTarget PRIVATE MyDep1 MyDep2 VolViz::Visualizer)
```

## Example Code
```C++
#include <VolViz/VolViz.h>

int main(int, char **) {
  // For physical units
  using namespace VolViz::literals;
  
  // load volume
  ...
  
  // load mesh into V and T (vertices and triangles)
  ...
  
  auto visualizer = VolViz::Visualizer{};
  
  // Set the physical scale you like to visualize, e.g. 1 unit equals 1cm
  visualizer.scale = 1_cm;
  
  // set volume
  // TODO: Not implemented, yet
  
  // set mesh
  visualizer.setMesh(V, T);
  
  // Opens the visualization window
  visualizer.start();
  
  // main update loop
  while (visualizer) visualizer.renderOneFrame();
  
  return EXIT_SUCCESS;
}
```

# Documentation
The [API documentation](https://ithron.github.io/VolViz/html) can be found [here](https://ithron.github.io/VolViz/html)
