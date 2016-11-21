#!/bin/sh

set -e

mkdir -p Dependencies/build

cd Dependencies/build

# Build GLFW
mkdir -p glfw
cd glfw
cmake ../../glfw -DCMAKE_BUILD_TYPE=Release -GXcode
cd ..

# # Build googletest
# mkdir -p googletest
# cd googletest
# cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GTEST=On -DBUILD_GMOCK=Off -GXcode ../../googletest
