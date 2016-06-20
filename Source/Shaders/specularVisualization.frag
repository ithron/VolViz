R"(
#version 410 core

uniform sampler2D tex;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
  vec2 specComponents = texture(tex, texcoord).ba;
  float specularity = specComponents.x;
  float shininess = specComponents.y;

  color.b = specularity;
  color.r = shininess / (shininess + 1.0);
  color.g = 0.0;
  color.a = 1.0;
}

)"
