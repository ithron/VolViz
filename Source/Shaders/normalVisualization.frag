R"(
#version 410 core

uniform sampler2D tex;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  vec3 normal = normalize(texture(tex, texcoord).rgb);

  color.rgb = (1.0 + normal) / 2.0;
}

)"
