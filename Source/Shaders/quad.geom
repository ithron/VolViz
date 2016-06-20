R"(
#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform vec2 topLeft;
uniform vec2 size;

out vec2 texcoord;

void main() {
  gl_Position = vec4(topLeft.x, topLeft.y, 0.0, 1.0);
  texcoord = vec2(0.0, 1.0);
  EmitVertex();

  gl_Position = vec4(topLeft.x, topLeft.y - size.y, 0.0, 1.0);
  texcoord = vec2(0.0, 0.0);
  EmitVertex();

  gl_Position = vec4(topLeft.x + size.x, topLeft.y, 0.0, 1.0);
  texcoord = vec2(1.0, 1.0);
  EmitVertex();

  gl_Position = vec4(topLeft.x + size.x, topLeft.y - size.y, 0.0, 1.0);
  texcoord = vec2(1.0, 0.0);
  EmitVertex();

  EndPrimitive();
}

)"
