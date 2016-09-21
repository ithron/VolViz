
R"(

#version 410 core

uniform vec4 position;
uniform vec3 pointColor;
uniform float size;

layout(location = 0) out vec3 color;


void main() {
  gl_Position = position;
  color = pointColor;
  gl_PointSize = size;
}

)"
