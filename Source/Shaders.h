#ifndef VolViz_Shaders_h
#define VolViz_Shaders_h

#include <string>

namespace VolViz {
namespace GL {
namespace Shaders {

extern std::string const nullVertShaderSrc;

extern std::string const fullscreenQuadGeomShaderSrc;

extern std::string const simpleTextureFragShaderSrc;

extern std::string const simpleVertShaderSrc;

extern std::string const passThroughFragShaderSrc;

extern std::string const gridGeometryShaderSrc;

extern std::string const deferredVertexShaderSrc;

extern std::string const deferredPassthroughFragShaderSrc;

extern std::string const normalVisualizationFragShaderSrc;

} // namespace Shaders
} // namespace GL
} // namespace VolViz

#endif // VolViz_Shaders_h
