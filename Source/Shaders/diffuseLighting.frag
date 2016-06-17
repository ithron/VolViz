R"(

#version 410 core

layout(location = 0) in vec2 texcoord;

uniform sampler2D normalAndSpecularTex;
uniform sampler2D depthTex;
uniform sampler2D albedoTex;

layout(location = 0) out vec4 color;

uniform vec4 lightPosition;
uniform vec3 lightColor;
uniform float lightAttenuation;

void main() {
  // Read values from textures
  vec2 normalUV = texture(normalAndSpecularTex, texcoord).xy;
  vec3 normal = vec3(normalUV, sqrt(1.0 - dot(normalUV, normalUV)));
  float depth = texture(depthTex, texcoord).r;
  vec3 albedo = texture(albedoTex, texcoord).rgb;

  // compute 3D position
  // use texcoord as transformed view space coordinates
  vec3 position = vec3(texcoord.x * 2.0 - 1.0, texcoord.y * 2 - 1.0, depth);

  // compute light direction. If the 4th component of lightPosition is 0,
  // the light is located infinitely away
  vec3 lightDirection;
  float att = 1.0;
  if (lightPosition.w < 0.1) {
    lightDirection = normalize(lightPosition.xyz);
  } else {
    vec3 posToLight = lightPosition.xyz - position;
    lightDirection = normalize(posToLight);
    float dist = length(posToLight);

    att = 1.0 / (1.0 + lightAttenuation * dist * dist);
  }

  float lightDotNormal = dot(lightDirection, normal);
  vec3 diffuseColor = lightColor * max(0.0, lightDotNormal);

  color.rgb = att * albedo * diffuseColor;
}

)"
