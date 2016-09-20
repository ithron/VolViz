#include "Shaders.h"
#include "GL/ShaderProgram.h"
#include "GL/Shaders.h"

namespace VolViz {
namespace Private_ {

GL::ShaderProgram &Shaders::operator[](std::string name) {
  auto search = shaders_.find(name);
  Expects(search != shaders_.end());

  return search->second;
}

void Shaders::init() {
  Expects(shaders_.empty());

  // Quad shader
  shaders_.emplace(
      "quad",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
              GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::simpleTextureFragShaderSrc))
          .link());

  // HDR quad shader
  shaders_.emplace(
      "hdrQuad",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
              GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::hdrTextureFragShaderSrc))
          .link());

  // Normal quad shader
  shaders_.emplace("normalQuad",
                   GL::ShaderProgram()
                       .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                                GL::Shaders::nullVertShaderSrc))
                       .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                                GL::Shaders::quadGeomShaderSrc))
                       .attachShader(GL::Shader(
                           GL_FRAGMENT_SHADER,
                           GL::Shaders::normalVisualizationFragShaderSrc))
                       .link());

  // Depth quad shader
  shaders_.emplace(
      "depthQuad",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
              GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(
              GL_FRAGMENT_SHADER, GL::Shaders::depthVisualizationFragShaderSrc))
          .link());

  // specular quad shader
  shaders_.emplace("specularQuad",
                   GL::ShaderProgram()
                       .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                                GL::Shaders::nullVertShaderSrc))
                       .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                                GL::Shaders::quadGeomShaderSrc))
                       .attachShader(GL::Shader(
                           GL_FRAGMENT_SHADER,
                           GL::Shaders::specularVisualizationFragShaderSrc))
                       .link());

  // Ambient pass shader
  shaders_.emplace(
      "ambientPass",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
              GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::ambientPassFragShaderSrc))
          .link());

  // Diffuse lighting pass
  shaders_.emplace("diffuseLightingPass",
                   GL::ShaderProgram()
                       .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                                GL::Shaders::nullVertShaderSrc))
                       .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                                GL::Shaders::quadGeomShaderSrc))
                       .attachShader(GL::Shader(
                           GL_FRAGMENT_SHADER,
                           GL::Shaders::diffuseLightingPassFragShaderSrc))
                       .link());

  // Specular lighting pass
  shaders_.emplace("specularLightingPass",
                   GL::ShaderProgram()
                       .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                                GL::Shaders::nullVertShaderSrc))
                       .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                                GL::Shaders::quadGeomShaderSrc))
                       .attachShader(GL::Shader(
                           GL_FRAGMENT_SHADER,
                           GL::Shaders::specularLightingPassFragShaderSrc))
                       .link());

  // Geometry stage shader
  shaders_.emplace(
      "geometryStage",
      GL::ShaderProgram()
          .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                   GL::Shaders::deferredVertexShaderSrc))
          .attachShader(
              GL::Shader(GL_FRAGMENT_SHADER,
                         GL::Shaders::deferredPassthroughFragShaderSrc))
          .link());

  // Grid shader
  shaders_.emplace(
      "grid",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                   GL::Shaders::gridGeometryShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::passThroughFragShaderSrc))
          .link());

  // Plane shader
  shaders_.emplace(
      "plane", GL::ShaderProgram()
                   .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                            GL::Shaders::nullVertShaderSrc))
                   .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                            GL::Shaders::planeGeomShaderSrc))
                   .attachShader(GL::Shader(
                       GL_FRAGMENT_SHADER,
                       GL::Shaders::deferredPassthroughFragShaderSrc))
                   .link());

  // BBox shader
  shaders_.emplace(
      "bbox",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                   GL::Shaders::bboxGeometryShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::passThroughFragShaderSrc))
          .link());

  // Selection index visualization shader
  shaders_.emplace(
      "selectionIndexVisualization",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
              GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(
              GL::Shader(GL_FRAGMENT_SHADER,
                         GL::Shaders::selectionIndexVisualizationFragShaderSrc))
          .link());

  // Point shader
  shaders_.emplace(
      "point",
      GL::ShaderProgram()
          .attachShader(
              GL::Shader(GL_VERTEX_SHADER, GL::Shaders::pointVertShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::passThroughFragShaderSrc))
          .link());
}

} // namespace Private_
} // namespace VolViz
