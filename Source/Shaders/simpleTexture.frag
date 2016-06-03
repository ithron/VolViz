R"(
#version 430 core

layout(location = 0) uniform sampler2D texture;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  color = texture2D(texture, texcoord);
}

)"
