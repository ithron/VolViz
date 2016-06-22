R"(
#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat3 inverseModelViewMatrix;
uniform vec3 color;
uniform float shininess;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 albedo;
layout(location = 2) out float specular;
layout(location = 3) out float gShininess;
layout(location = 4) out vec3 texcoord;

void main() {
  const vec4 points[4] = vec4[4](
    vec4(-1.0, 1.0, 0.0, 1.0),
    vec4(-1.0, -1.0, 0.0, 1.0),
    vec4(1.0, 1.0, 0.0, 1.0),
    vec4(1.0, -1.0, 0.0, 1.0)
  );
  const vec3 normZ = vec3(0.0, 0.0, 1.0);

  normal = normalize(inverseModelViewMatrix * normZ);
  albedo = color;
  specular = 1.0;
  gShininess = shininess;

  gl_Position = modelViewProjectionMatrix * points[0];
  texcoord = (modelViewMatrix * points[0]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[1];
  texcoord = (modelViewMatrix * points[1]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[2];
  texcoord = (modelViewMatrix * points[2]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[3];
  texcoord = (modelViewMatrix * points[3]).xyz;
  EmitVertex();

  EndPrimitive();
}

)"
