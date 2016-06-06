R"(
#version 410 core

const uint size = 31;

layout(points) in;
layout(line_strip, max_vertices = 124 /* 4 * size */) out;

layout(location = 0) out vec4 color;

/* layout(location = 0) in vec4 origin; */

uniform float scale;
uniform mat4 viewProjectionMatrix;

void main() {
  vec4 p;
  const float halfSize = float(size/2);
  const int halfSizei = int(size / 2);
  float minCoord = -scale * halfSize;
  float maxCoord = scale * halfSize;

  /* for (int i = -int(size/2); i <= int(size/2); ++i) { */
  for (int i = -halfSizei; i <= halfSizei; ++i) {
    // Horizontal grid line
    p = vec4(minCoord, scale * float(i), 0.0, 1.0);
    gl_Position = viewProjectionMatrix * p;
    color = vec4(1.0, 1.0, 1.0, 1.0);
    EmitVertex();

    p = vec4(maxCoord, scale * float(i), 0.0, 1.0);
    gl_Position = viewProjectionMatrix * p;
    EmitVertex();
    EndPrimitive();

    // Vertical grid line
    p = vec4(scale * float(i), minCoord, 0.0, 1.0);
    gl_Position = viewProjectionMatrix * p;
    color = vec4(1.0, 1.0, 1.0, 1.0);
    EmitVertex();

    p = vec4(scale * float(i), maxCoord, 0.0, 1.0);
    gl_Position = viewProjectionMatrix * p;
    color = vec4(1.0, 1.0, 1.0, 1.0);
    EmitVertex();
    EndPrimitive();
  }
}

)"
