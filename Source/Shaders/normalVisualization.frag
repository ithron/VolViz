R"(
#version 410 core

uniform sampler2D tex;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  vec2 normalUV = texture(tex, texcoord).xy;
  float z = sqrt(1.0 - dot(normalUV, normalUV));
  vec3 normal = vec3(normalUV, z);

  color.rgb = (1.0 + normal) / 2.0;
}

)"
