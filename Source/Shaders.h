#pragma once

#include "GL/ShaderProgram.h"
#include "Types.h"

#include <unordered_map>

namespace VolViz {
namespace Private_ {

namespace GL {
class ShaderProgram;
} // namespace GL

class Shaders {
public:
  GL::ShaderProgram &operator[](std::string name);

  /// Compiles and links all shaders. Must be called once
  void init();

private:
  using ShaderProgramTable = std::unordered_map<std::string, GL::ShaderProgram>;

  ShaderProgramTable shaders_;
};

} // namespace Private_
} // namespace VolViz
