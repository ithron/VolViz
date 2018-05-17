#ifndef VolViz_Shaders_h
#define VolViz_Shaders_h

#include <string>

namespace VolViz {
namespace Private_ {
namespace GL {
namespace Shaders {

extern std::string const ambientPassFragShaderSrc;
extern std::string const bboxGeometryShaderSrc;
extern std::string const coloredQuadFragmentShaderSrc;
extern std::string const deferredPassthroughFragShaderSrc;
extern std::string const deferredVertexShaderSrc;
extern std::string const depthVisualizationFragShaderSrc;
extern std::string const diffuseLightingPassFragShaderSrc;
extern std::string const gridGeometryShaderSrc;
extern std::string const hdrTextureFragShaderSrc;
extern std::string const normalVisualizationFragShaderSrc;
extern std::string const nullVertShaderSrc;
extern std::string const passThroughFragShaderSrc;
extern std::string const planeGeomShaderSrc;
extern std::string const cubeGeomShaderSrc;
extern std::string const pointVertShaderSrc;
extern std::string const quadGeomShaderSrc;
extern std::string const selectionFragShaderSrc;
extern std::string const selectionIndexVisualizationFragShaderSrc;
extern std::string const simpleTextureFragShaderSrc;
extern std::string const simpleVertShaderSrc;
extern std::string const specularLightingPassFragShaderSrc;
extern std::string const specularVisualizationFragShaderSrc;

} // namespace Shaders
} // namespace GL
} // namespace Private_
} // namespace VolViz

#endif // VolViz_Shaders_h
