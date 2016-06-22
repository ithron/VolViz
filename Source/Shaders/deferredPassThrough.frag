R"(

#version 410 core

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 albedo;
layout(location = 2) in float specular;
layout(location = 3) in float gShininess;

layout(location = 0) out vec4 gNormalAndSpecular;
layout(location = 1) out vec4 gAlbedo;

void main() {
  gNormalAndSpecular = vec4(normalize(normal).xy, specular, gShininess);
  gAlbedo = vec4(albedo, 1.0);
}

)"