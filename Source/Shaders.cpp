#include "Shaders.h"

namespace VolViz {
namespace GL {
namespace Shaders {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
std::string const nullVertShaderSrc =
#include "Shaders/null.vert"
    ;
std::string const fullscreenQuadGeomShaderSrc =
#include "Shaders/fullscreenQuad.geom"
    ;
std::string const simpleTextureFragShaderSrc =
#include "Shaders/simpleTexture.frag"
    ;
std::string const simpleVertShaderSrc =
#include "Shaders/simple.vert"
    ;
std::string const passThroughFragShaderSrc =
#include "Shaders/passThrough.frag"
    ;

std::string const gridGeometryShaderSrc =
#include "Shaders/grid.geom"
    ;
#pragma clang diagnostic pop

} // namespace Shaders
} // namespace GL

} // namespace VolViz
