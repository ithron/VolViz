R"(
#version 410 core

uniform uint nObjects;
uniform usampler2D tex;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  uint index = texture(tex, texcoord).r;
  uint indexColorBoundary = nObjects / 3;

  if (index <= indexColorBoundary) {
    color.r = float(index) / float(indexColorBoundary);
  } else if (index <= 2 * indexColorBoundary) {
    color.g = float(index - indexColorBoundary) / float(indexColorBoundary);
  } else {
    color.b = float(index - 2 * indexColorBoundary) / float(indexColorBoundary);
  }
}

)"
