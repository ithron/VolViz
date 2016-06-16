R"(

#version 410 core

layout(location = 0) in vec2 texcoord;

uniform sampler2D normalAndSpecularTex;
uniform sampler2D depthTex;
uniform sampler2D albedoTex;

layout(location = 0) out vec4 color;

uniform vec4 lightPosition;
uniform vec3 lightColor;

void main() {
  // Read values from textures
  vec4 normalAndSpecular = texture(normalAndSpecularTex, texcoord);
  vec2 normalUV = normalAndSpecular.xy;
  vec3 normal = vec3(normalUV, sqrt(1.0 - dot(normalUV, normalUV)));
  float depth = texture(depthTex, texcoord).r;
  vec3 albedo = texture(albedoTex, texcoord).rgb;
  
  vec3 specular = vec3(normalAndSpecular.z);
  float shininess = normalAndSpecular.w;
  // compute 3D position
  // use texcoord as transformed view space coordinates
  vec3 position = vec3(texcoord.x * 2.0 - 1.0, texcoord.y * 2 - 1.0, depth);
  
  // compute light direction. If the 4th component of lightPosition is 0,
  // the light is located infinitely away
  vec3 lightDirection;
  if (lightPosition.w < 0.1) {
    lightDirection = normalize(lightPosition.xyz);
  } else {
    lightDirection = normalize(lightPosition.xyz - position);
  }

  vec3 directionToCamera = vec3(0.0, 0.0, 1.0);
  float lightDotNormal = dot(lightDirection, normal);
//   vec3 reflectedRayDirection = 2 * lightDotNormal * normal - lightDirection;
  vec3 reflectedRayDirection = reflect(-lightDirection, normal);

  vec3 diffuseColor = albedo * max(0.0, lightDotNormal);
  vec3 specularColor =
     specular * pow(max(0.0, reflectedRayDirection.z), shininess);

  color.rgb = lightColor * (diffuseColor + specularColor);
}

)"
