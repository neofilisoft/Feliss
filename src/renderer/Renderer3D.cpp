#include "renderer/Renderer3D.h"

#include "renderer/RenderPipeline.h"

namespace Feliss {

Renderer3D::Renderer3D(RenderPipeline& pipeline)
    : m_pipeline(pipeline) {}

void Renderer3D::drawMesh(AssetID mesh,
                          AssetID material,
                          const Mat4& transform,
                          bool castShadow,
                          u32 layer) {
    m_pipeline.submitMesh(mesh, material, transform, castShadow, layer);
}

void Renderer3D::addDirectionalLight(const Color& color,
                                     f32 intensity,
                                     const Vec3& direction) {
    m_pipeline.submitLight(0, color, intensity, Vec3::zero(), direction, 0.0f);
}

void Renderer3D::addPointLight(const Color& color,
                               f32 intensity,
                               const Vec3& position,
                               f32 range) {
    m_pipeline.submitLight(1, color, intensity, position, Vec3 {0.0f, -1.0f, 0.0f}, range);
}

} // namespace Feliss
