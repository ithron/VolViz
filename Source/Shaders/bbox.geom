R"(
#version 410 core

layout(points) in;
layout(line_strip, max_vertices = 16) out;

uniform mat4 modelViewProjectionMatrix;
uniform vec3 lineColor;

out vec3 color;

void main() {
  vec4 corners[8] = vec4[8](
      // Top
      vec4(-0.5,  0.5, -0.5, 1.0),
      vec4(-0.5,  0.5,  0.5, 1.0),
      vec4( 0.5,  0.5,  0.5, 1.0),
      vec4( 0.5,  0.5, -0.5, 1.0),
      // Bottom
      vec4(-0.5, -0.5, -0.5, 1.0),
      vec4(-0.5, -0.5,  0.5, 1.0),
      vec4( 0.5, -0.5,  0.5, 1.0),
      vec4( 0.5, -0.5, -0.5, 1.0)
  );

  vec4 cornerPos[8] = vec4[8](
    modelViewProjectionMatrix * corners[0],
    modelViewProjectionMatrix * corners[1],
    modelViewProjectionMatrix * corners[2],
    modelViewProjectionMatrix * corners[3],
    modelViewProjectionMatrix * corners[4],
    modelViewProjectionMatrix * corners[5],
    modelViewProjectionMatrix * corners[6],
    modelViewProjectionMatrix * corners[7]
  );

  color = lineColor;

  // Top
  gl_Position = cornerPos[0];
  EmitVertex();

  gl_Position = cornerPos[1];
  EmitVertex();

  gl_Position = cornerPos[2];
  EmitVertex();

  gl_Position = cornerPos[3];
  EmitVertex();

  gl_Position = cornerPos[0];
  EmitVertex();
  // Bottom

  gl_Position = cornerPos[4];
  EmitVertex();

  gl_Position = cornerPos[5];
  EmitVertex();

  gl_Position = cornerPos[6];
  EmitVertex();

  gl_Position = cornerPos[7];
  EmitVertex();

  gl_Position = cornerPos[4];
  EmitVertex();

  EndPrimitive();

  // connection between top and bottom, 0 and 4 are connected, already
  gl_Position = cornerPos[1];
  EmitVertex();

  gl_Position = cornerPos[5];
  EmitVertex();

  EndPrimitive();

  gl_Position = cornerPos[2];
  EmitVertex();

  gl_Position = cornerPos[6];
  EmitVertex();

  EndPrimitive();

  gl_Position = cornerPos[3];
  EmitVertex();

  gl_Position = cornerPos[7];
  EmitVertex();

  EndPrimitive();
}

)"
