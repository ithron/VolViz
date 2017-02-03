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
  vec4 normalAndSpecular = texture(normalAndSpecularTex, texcoord);
  vec3 albedo = texture(albedoTex, texcoord).rgb;
  vec4 idx = texture(indexTex, texcoord).rgba;
  float isForeGround = max(1.0, idx.r + idx.g + idx.b + idx.a);

  vec2 normalUV = normalAndSpecular.xy;
  vec3 specular = vec3(normalAndSpecular.z);
  float shininess = normalAndSpecular.w;

  vec3 normal = vec3(normalUV, sqrt(1.0 - dot(normalUV, normalUV)));
  vec3 lightDirection = normalize(lightPosition.xyz);

  float lightDotNormal = dot(lightDirection, normal);
  vec3 reflectedRayDirection = reflect(-lightDirection, normal);

  // since all vectors are in view space, the view direction is just along the
  // -z axis. Therefore dot(reflectoredRayDirection, -viewDirection) is just
  // reflectedRatDirection.z
  vec3 specularColor =
     lightColor * specular *
       min(isForeGround, pow(max(0.0, reflectedRayDirection.z), shininess));

  color.rgb = albedo * (1.0 - isForeGround + specularColor);
}

)"
