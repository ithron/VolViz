R"(
#version 410 core

in vec2 texcoord;

uniform sampler2D tex;
uniform float ambientFactor;

layout(location = 0) out vec4 color;

void main() {
  color = vec4(texture(tex, texcoord).rgb * ambientFactor, 1.0);
}

)"
