R"(
#version 410 core

uniform sampler2D tex;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  color = texture(tex, texcoord);
}

)"
