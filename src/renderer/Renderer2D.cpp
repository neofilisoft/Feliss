#include "renderer/Renderer2D.h"

#include "renderer/RenderPipeline.h"

namespace Feliss {

Renderer2D::Renderer2D(RenderPipeline& pipeline)
    : m_pipeline(pipeline) {}

void Renderer2D::drawSprite(AssetID texture,
                            const Color& tint,
                            const Vec2& size,
                            const Mat4& transform,
                            int sortOrder) {
    m_pipeline.submitSprite(texture, tint, size, transform, sortOrder);
}

void Renderer2D::drawQuad(const Mat4& transform,
                          const Vec2& size,
                          const Color& tint,
                          int sortOrder) {
    drawSprite(NULL_ASSET, tint, size, transform, sortOrder);
}

} // namespace Feliss
