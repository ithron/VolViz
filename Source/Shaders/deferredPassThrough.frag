R"(

#version 410 core

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 albedo;
layout(location = 2) in vec4 specular;

layout(location = 0) out vec4 gNormal;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gSpecular;

void main() {
  gNormal = vec4(normalize(normal), 1.0);
  gAlbedo = vec4(albedo, 1.0);
  gSpecular = specular;
}

)"
