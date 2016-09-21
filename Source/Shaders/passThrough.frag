R"(

#version 410 core

layout (location = 0) in vec3 color;

layout(location = 0) out vec4 colorOut;

void main() {
  colorOut.rgb = color;
}

)"
