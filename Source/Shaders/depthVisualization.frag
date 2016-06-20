R"(
#version 410 core

uniform sampler2D tex;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  float depth = 1.0 - texture(tex, texcoord).r;

  color = vec4(depth, depth, depth, 1.0);
}

)"
