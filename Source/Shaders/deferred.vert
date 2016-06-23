R"(

#version 410 core

uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 inverseModelViewMatrix;
uniform mat3 textureTransformMatrix;
uniform float shininess;

layout(location = 0) in vec4 positionIn;
layout(location = 1) in vec3 normalIn;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 albedo;
layout(location = 2) out float specular;
layout(location = 3) out float gShininess;
layout(location = 4) vec3 texcoord;


void main() {
  vec4 posInViewSpace = modelViewMatrix * positionIn
  gl_Position = modelViewProjectionMatrix * positionIn;
  // Convert normal to view space
  // normal = normalize(transpose(inverseModelViewMatrix) * normalIn);
  normal = normalize(inverseModelViewMatrix * normalIn);
  // set fixed colors for now
  albedo = vec3(0.8);
  specular = 1.0;
  
  gShininess = shininess;

  texcoord = (textureTransformMatrix * posInViewSpace).xyz;
}

)"
