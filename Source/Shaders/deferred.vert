R"(

#version 410 core

uniform mat4 modelViewProjectionMatrix;
uniform mat3 inverseModelViewMatrix;
uniform float shininess;

layout(location = 0) in vec4 positionIn;
layout(location = 1) in vec3 normalIn;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 albedo;
layout(location = 2) out vec4 specular;

void main() {
  gl_Position = modelViewProjectionMatrix * positionIn;
  // Convert normal to view space
  // normal = normalize(transpose(inverseModelViewMatrix) * normalIn);
  normal = normalize(inverseModelViewMatrix * normalIn);
  // set fixed colors for now
  albedo = vec3(1.0, 1.0, 1.0);
  specular = vec4(1.0, 1.0, 1.0, shininess);
}

)"
