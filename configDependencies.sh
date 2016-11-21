#!/bin/sh

set -e

echo Configure VolViz

PROJECT_ROOT=`pwd`
mkdir -p $PROJECT_ROOT/Dependencies/build


# Build GLFW
if [ ! -d "$PROJECT_ROOT/Dependencies/build/glfw" ]; then
  cd Dependencies/build
  mkdir -p glfw
  cd glfw
  cmake ../../glfw -DCMAKE_BUILD_TYPE=Release -GXcode
else
  echo "Skip glfw"
fi

cd $PROJECT_ROOT
