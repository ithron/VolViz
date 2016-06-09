R"(

#version 410 core

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 albedo;
layout(location = 2) in vec3 specular;

layout(location = 0) out vec4 gNormal;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gSpecular;

void main() {
  gNormal.rgb = normalize(normal);
  gNormal.a = 1.0;
  gAlbedo.rgb = albedo;
  gAlbedo.a = 1.0;
  gSpecular.rgb = specular;
  gSpecular.a = 1.0;
}

)"
