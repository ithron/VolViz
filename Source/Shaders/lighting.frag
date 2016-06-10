R"(

#version 410 core

layout(location = 0) in vec2 texcoord;

uniform sampler2D normalTex;
uniform sampler2D depthTex;
uniform sampler2D albedoTex;
uniform sampler2D specularTex;

layout(location = 0) out vec4 color;

uniform vec4 lightPosition;
uniform vec3 lightColor;

void main() {
  // Read values from textures
  vec3 normal = normalize(texture(normalTex, texcoord).rgb);
  float depth = texture(depthTex, texcoord).r;
  vec3 albedo = texture(albedoTex, texcoord).rgb;
  vec4 specShininess = texture(specularTex, texcoord);
  vec3 specular = specShininess.rgb;
  float shininess = specShininess.a;
  // compute 3D position
  // use texcoord as transformed view space coordinates
  vec3 position = vec3(texcoord.x * 2.0 - 1.0, texcoord.y * 2 - 1.0, depth);

  // compute light direction. If the 4th component of lightPosition is 0,
  // the light is located infinitely away
  vec3 lightDirection;
  if (lightPosition.w < 0.5) {
    lightDirection = normalize(lightPosition.xyz);
  } else {
    lightDirection = normalize(lightPosition.xyz - position);
  }

  vec3 directionToCamera = vec3(0.0, 0.0, 1.0);
  float lightDotNormal = dot(lightDirection, normal);
  // vec3 reflectedRayDirection = 2 * lightDotNormal * normal - lightDirection;
  vec3 reflectedRayDirection = reflect(-lightDirection, normal);

  vec3 diffuseColor = albedo * max(0.0, lightDotNormal);
  vec3 specularColor =
    specular * max(0.0,
        pow(dot(reflectedRayDirection, directionToCamera), shininess));

  color = clamp(vec4(lightColor * (diffuseColor + specularColor), 1.0), 0.0, 1.0);
}

)"
