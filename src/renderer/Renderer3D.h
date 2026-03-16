#pragma once

#include "feliss/Types.h"

namespace Feliss {

class RenderPipeline;

class Renderer3D {
public:
    explicit Renderer3D(RenderPipeline& pipeline);

    void drawMesh(AssetID mesh,
                  AssetID material,
                  const Mat4& transform,
                  bool castShadow = true,
                  u32 layer = 1);

    void addDirectionalLight(const Color& color,
                             f32 intensity,
                             const Vec3& direction);

    void addPointLight(const Color& color,
                       f32 intensity,
                       const Vec3& position,
                       f32 range);

private:
    RenderPipeline& m_pipeline;
};

} // namespace Feliss
