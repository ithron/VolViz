#include "Shaders.h"
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
      std::move(
          GL::ShaderProgram()
              .attachShader(
                  GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
              .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                       GL::Shaders::quadGeomShaderSrc))
              .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                       GL::Shaders::simpleTextureFragShaderSrc))
              .link()));

  // HDR quad shader
  shaders_.emplace(
      "hdrQuad",
      std::move(
          GL::ShaderProgram()
              .attachShader(
                  GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
              .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                       GL::Shaders::quadGeomShaderSrc))
              .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                       GL::Shaders::hdrTextureFragShaderSrc))
              .link()));

  // Normal quad shader
  shaders_.emplace(
      "normalQuad",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::normalVisualizationFragShaderSrc))
                    .link()));

  // Depth quad shader
  shaders_.emplace(
      "depthQuad",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::depthVisualizationFragShaderSrc))
                    .link()));

  // specular quad shader
  shaders_.emplace(
      "specularQuad",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::specularVisualizationFragShaderSrc))
                    .link()));

  // Ambient pass shader
  shaders_.emplace(
      "ambientPass",
      std::move(
          GL::ShaderProgram()
              .attachShader(
                  GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
              .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                       GL::Shaders::quadGeomShaderSrc))
              .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                       GL::Shaders::ambientPassFragShaderSrc))
              .link()));

  // Diffuse lighting pass
  shaders_.emplace(
      "diffuseLightingPass",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::diffuseLightingPassFragShaderSrc))
                    .link()));

  // Specular lighting pass
  shaders_.emplace(
      "specularLightingPass",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::specularLightingPassFragShaderSrc))
                    .link()));

  // Geometry stage shader
  shaders_.emplace(
      "geometryStage",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(
                        GL_VERTEX_SHADER, GL::Shaders::deferredVertexShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::deferredPassthroughFragShaderSrc))
                    .link()));

  // Grid shader
  shaders_.emplace(
      "grid",
      std::move(
          GL::ShaderProgram()
              .attachShader(
                  GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
              .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                       GL::Shaders::gridGeometryShaderSrc))
              .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                       GL::Shaders::passThroughFragShaderSrc))
              .link()));

  // Plane shader
  shaders_.emplace(
      "plane",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::planeGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::deferredPassthroughFragShaderSrc))
                    .link()));

  // BBox shader
  shaders_.emplace(
      "bbox",
      std::move(
          GL::ShaderProgram()
              .attachShader(
                  GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
              .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                       GL::Shaders::bboxGeometryShaderSrc))
              .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                       GL::Shaders::passThroughFragShaderSrc))
              .link()));

  // Selection index visualization shader
  shaders_.emplace(
      "selectionIndexVisualization",
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::selectionIndexVisualizationFragShaderSrc))
                    .link()));

  // Point shader
  shaders_.emplace(
      "point",
      std::move(
          GL::ShaderProgram()
              .attachShader(
                  GL::Shader(GL_VERTEX_SHADER, GL::Shaders::pointVertShaderSrc))
              .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                       GL::Shaders::passThroughFragShaderSrc))
              .link()));
}

} // namespace Private_
} // namespace VolViz
