R"(

#version 410 core

uniform sampler3D volume;
uniform uint index;
uniform bool isGray;
uniform vec2 range;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 albedo;
layout(location = 2) in float specular;
layout(location = 3) in float gShininess;
layout(location = 4) in vec3 texcoord;

layout(location = 0) out vec4 gNormalAndSpecular;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out uint gIndex;

void main() {
  vec3 volColor;
  if (isGray) {
    // If texture is gray scale, use a window/level approach to scale the range
    // down to [0, 1]
    float intensity = texture(volume, texcoord).r;
    volColor =
      vec3(clamp((intensity - range.x) / (range.y - range.x), 0.0, 1.0));
  } else {
    // If texture is colored, the colors can be used directly
    volColor = texture(volume, texcoord).rgb;
  }

  gNormalAndSpecular = vec4(normalize(normal).xy, specular, gShininess);
  gAlbedo = vec4(albedo * volColor, 1.0);
  gIndex = index;
}

)"
