R"(

#version 430 core

layout(location = 0) uniform mat4 modelViewProjectionMatrix;

layout(location = 0) in vec4 position;
//layout(location = 1) in vec4 color;

layout(location = 0) out vec4 vertexColor;

void main() {
  vec4 p = modelViewProjectionMatrix * position;
  gl_Position = p;
  // vertexColor = color;
  vertexColor = (1.0 + p) / 2.0;
  //vertexColor = vec4(1.0, 1.0, 0.0, 1.0);
}

)"
