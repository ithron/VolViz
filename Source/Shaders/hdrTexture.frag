R"(
#version 410 core

uniform sampler2D tex;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  vec4 colorIn = texture(tex, texcoord);
  color = colorIn / (colorIn + 1.0);
}

)"
