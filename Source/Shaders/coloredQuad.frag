R"(
#version 410 core

uniform vec3 colorIn;

layout(location = 0) out vec4 color;

in vec3 texcoord;

void main() {
  color = vec4(colorIn, 1.0);
}

)"
