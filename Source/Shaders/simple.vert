R"(

#version 410 core

uniform mat4 modelViewProjectionMatrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
//layout(location = 1) in vec4 color;

layout(location = 0) out vec4 color;

void main() {
  vec4 p = modelViewProjectionMatrix * position;
  gl_Position = p;
  // gl_Position = position;
  // vertexColor = color;
  // color = (1.0 + p) / 2.0;
  color = vec4(1.0, 0.0, 0.0, 1.0);
  //vertexColor = vec4(1.0, 1.0, 0.0, 1.0);
}

)"
