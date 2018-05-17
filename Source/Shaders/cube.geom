R"(
#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 inverseModelViewMatrix;
uniform mat4 textureTransformMatrix;
uniform vec3 color;
uniform float shininess;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 albedo;
layout(location = 2) out float specular;
layout(location = 3) out float gShininess;
layout(location = 4) out vec3 texcoord;

void main() {
  const vec4 points[8] = vec4[8](
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(-1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, -1.0, 1.0),
    vec4(-1.0, 1.0, -1.0, 1.0),
    vec4(1.0, -1.0, 1.0, 1.0),
    vec4(-1.0, -1.0, 1.0, 1.0),
    vec4(-1.0, -1.0, -1.0, 1.0),
    vec4(1.0, -1.0, -1.0, 1.0)
  );
  const vec3 normX = vec3(1.0, 0.0, 0.0);
  const vec3 normY = vec3(0.0, 1.0, 0.0);
  const vec3 normZ = vec3(0.0, 0.0, 1.0);

  vec3 normPX = normalize(inverseModelViewMatrix * normX);
  vec3 normNX = normalize(inverseModelViewMatrix * -normX);
  vec3 normPY = normalize(inverseModelViewMatrix * normY);
  vec3 normNY = normalize(inverseModelViewMatrix * -normY);
  vec3 normPZ = normalize(inverseModelViewMatrix * normZ);
  vec3 normNZ = normalize(inverseModelViewMatrix * -normZ);

  albedo = color;
  specular = 1.0;
  gShininess = shininess;

  // back plane (in negative Z direction_
  normal = normNZ;
  gl_Position = modelViewProjectionMatrix * points[3];
  texcoord = (textureTransformMatrix * modelMatrix * points[3]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[2];
  texcoord = (textureTransformMatrix * modelMatrix * points[2]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[6];
  texcoord = (textureTransformMatrix * modelMatrix * points[6]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[7];
  texcoord = (textureTransformMatrix * modelMatrix * points[7]).xyz;
  EmitVertex();

  EndPrimitive();

  // front plane (in positive z direction)
  normal = normPZ;
  gl_Position = modelViewProjectionMatrix * points[5];
  texcoord = (textureTransformMatrix * modelMatrix * points[5]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[4];
  texcoord = (textureTransformMatrix * modelMatrix * points[4]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[1];
  texcoord = (textureTransformMatrix * modelMatrix * points[1]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[0];
  texcoord = (textureTransformMatrix * modelMatrix * points[0]).xyz;
  EmitVertex();

  EndPrimitive();

  // right plane (in positive x direction)
  normal = normPX;
  gl_Position = modelViewProjectionMatrix * points[7];
  texcoord = (textureTransformMatrix * modelMatrix * points[7]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[4];
  texcoord = (textureTransformMatrix * modelMatrix * points[4]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[2];
  texcoord = (textureTransformMatrix * modelMatrix * points[2]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[0];
  texcoord = (textureTransformMatrix * modelMatrix * points[0]).xyz;
  EmitVertex();

  EndPrimitive();

  // left plane (in negative x direction)
  normal = normNX;
  gl_Position = modelViewProjectionMatrix * points[3];
  texcoord = (textureTransformMatrix * modelMatrix * points[3]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[6];
  texcoord = (textureTransformMatrix * modelMatrix * points[6]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[1];
  texcoord = (textureTransformMatrix * modelMatrix * points[1]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[5];
  texcoord = (textureTransformMatrix * modelMatrix * points[5]).xyz;
  EmitVertex();

  EndPrimitive();

  // top plane (in positive y direction)
  normal = normPY;
  gl_Position = modelViewProjectionMatrix * points[2];
  texcoord = (textureTransformMatrix * modelMatrix * points[2]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[0];
  texcoord = (textureTransformMatrix * modelMatrix * points[0]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[3];
  texcoord = (textureTransformMatrix * modelMatrix * points[3]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[1];
  texcoord = (textureTransformMatrix * modelMatrix * points[1]).xyz;
  EmitVertex();

  EndPrimitive();

  // bottom plane (in negative y direction)
  normal = normNY;
  gl_Position = modelViewProjectionMatrix * points[7];
  texcoord = (textureTransformMatrix * modelMatrix * points[7]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[4];
  texcoord = (textureTransformMatrix * modelMatrix * points[4]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[6];
  texcoord = (textureTransformMatrix * modelMatrix * points[6]).xyz;
  EmitVertex();

  gl_Position = modelViewProjectionMatrix * points[5];
  texcoord = (textureTransformMatrix * modelMatrix * points[5]).xyz;
  EmitVertex();

  EndPrimitive();
}

)"
