R"(

#version 410 core

layout(location = 0) in vec2 texcoord;

uniform sampler2D normalAndSpecularTex;
uniform sampler2D albedoTex;
uniform sampler2D indexTex;

layout(location = 0) out vec4 color;

uniform vec3 lightPosition;
uniform vec3 lightColor;

void main() {
  // Read values from textures
  vec2 normalUV = texture(normalAndSpecularTex, texcoord).xy;
  vec3 normal = vec3(normalUV, sqrt(1.0 - dot(normalUV, normalUV)));
  vec3 albedo = texture(albedoTex, texcoord).rgb;
  vec4 idx = texture(indexTex, texcoord).rgba;
  float isForeGround = max(1.0, idx.r + idx.g + idx.b + idx.a);

  // compute light direction. Since only directional lights are supported,
  // the direction is equal to the normalized light position
  vec3 lightDirection = normalize(lightPosition);

  float lightDotNormal = dot(lightDirection, normal);
  vec3 diffuseColor = lightColor * min(isForeGround, max(0.0, lightDotNormal));

  color.rgb = albedo * (1.0 - isForeGround + diffuseColor);
}

)"
