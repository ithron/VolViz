R"(
#version 410 core

in vec2 texcoord;

uniform sampler2D tex;
uniform vec3 lightColor;

layout(location = 0) out vec4 color;

void main() {
  color = vec4(texture(tex, texcoord).rgb * lightColor, 1.0);
}

)"
