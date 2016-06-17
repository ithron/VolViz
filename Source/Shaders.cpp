#include "Shaders.h"

namespace VolViz {
namespace GL {
namespace Shaders {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
std::string const nullVertShaderSrc =
#include "Shaders/null.vert"
    ;
std::string const quadGeomShaderSrc =
#include "Shaders/quad.geom"
    ;
std::string const simpleTextureFragShaderSrc =
#include "Shaders/simpleTexture.frag"
    ;
std::string const hdrTextureFragShaderSrc =
#include "Shaders/hdrTexture.frag"
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

std::string const deferredVertexShaderSrc =
#include "Shaders/deferred.vert"
    ;

std::string const deferredPassthroughFragShaderSrc =
#include "Shaders/deferredPassThrough.frag"
    ;

std::string const normalVisualizationFragShaderSrc =
#include "Shaders/normalVisualization.frag"
    ;

std::string const depthVisualizationFragShaderSrc =
#include "Shaders/depthVisualization.frag"
    ;

std::string const ambientPassFragShaderSrc =
#include "Shaders/ambientPass.frag"
    ;

std::string const diffuseLightingPassFragShaderSrc =
#include "Shaders/diffuseLighting.frag"
    ;

std::string const specularVisualizationFragShaderSrc =
#include "Shaders/specularVisualization.frag"
    ;

#pragma clang diagnostic pop

} // namespace Shaders
} // namespace GL

} // namespace VolViz
