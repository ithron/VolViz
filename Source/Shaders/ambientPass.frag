R"(
#version 410 core

in vec2 texcoord;

uniform sampler2D tex;
uniform vec3 lightColor;
uniform sampler2D indexTex;

layout(location = 0) out vec4 color;

void main() {
  vec4 idx = texture(indexTex, texcoord).rgba;
  float isForeGround = max(1.0, idx.r + idx.g + idx.b + idx.a);
  vec3 albedo = texture(tex, texcoord).rgb;
  color = vec4(albedo * (1.0 - isForeGround + lightColor), 1.0);
}

)"
