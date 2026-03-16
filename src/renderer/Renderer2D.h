#pragma once

#include "feliss/Types.h"

namespace Feliss {

class RenderPipeline;

class Renderer2D {
public:
    explicit Renderer2D(RenderPipeline& pipeline);

    void drawSprite(AssetID texture,
                    const Color& tint,
                    const Vec2& size,
                    const Mat4& transform,
                    int sortOrder = 0);

    void drawQuad(const Mat4& transform,
                  const Vec2& size,
                  const Color& tint = Color::white(),
                  int sortOrder = 0);

private:
    RenderPipeline& m_pipeline;
};

} // namespace Feliss
