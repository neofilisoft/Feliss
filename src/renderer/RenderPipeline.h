#pragma once
#include "renderer/RenderTypes.h"
#include "renderer/IRendererBackend.h"
#include <vector>
#include <string>
#include <memory>

namespace Feliss {

class Window;

// =====================================================================
// RenderPipeline — owns the backend, manages passes & draw queue
// =====================================================================
struct RenderPipelineDesc {
    RenderAPI api    = RenderAPI::OpenGL;
    int       width  = 1280;
    int       height = 720;
    bool      vsync  = true;
    int       msaa   = 4;
};

class RenderPipeline {
public:
    explicit RenderPipeline(const RenderPipelineDesc& desc);
    ~RenderPipeline();
    RenderPipeline(const RenderPipeline&)            = delete;
    RenderPipeline& operator=(const RenderPipeline&) = delete;

    // Lifecycle
    bool init(Window& window);
    void shutdown();
    void onResize(int w, int h);

    // Frame
    void beginFrame();
    void renderScenePass();
    void presentFrame();
    void endFrame();
    void beginImGui();
    void endImGui();

    // Submit draw commands (called by World::render)
    void submitMesh(AssetID mesh, AssetID mat,
                    const Mat4& transform,
                    bool castShadow = true, u32 layer = 1);
    void submitSprite(AssetID tex, const Color& tint,
                      const Vec2& size, const Mat4& transform,
                      int sortOrder = 0);
    void submitLight(int lightType, const Color& color,
                     f32 intensity, const Vec3& pos,
                     const Vec3& dir, f32 range);
    void setCamera(const Mat4& viewProjection, const Color& clearColor);

    // Pass management
    void addPass(const RenderPass& pass);
    void removePass(const std::string& name);
    RenderPass* getPass(const std::string& name);
    const std::vector<RenderPass>& passes() const { return m_passes; }
    void setPassEnabled(const std::string& name, bool en);

    // Post-processing
    void addPostEffect(const PostEffect& fx);
    void removePostEffect(const std::string& name);
    void setPostEffectEnabled(const std::string& name, bool en);
    const std::vector<PostEffect>& postEffects() const { return m_postEffects; }

    // Render targets
    GpuHandle createRenderTarget(int w, int h,
                                  PixelFormat fmt = PixelFormat::RGBA8,
                                  bool hasDepth = true);
    void destroyRenderTarget(GpuHandle h);
    GpuHandle getRenderTargetColorTex(GpuHandle h);
    void setSceneViewportSize(int w, int h);
    GpuHandle sceneViewportTexture() const;

    // Pipeline mode
    void         setPipelineMode(PipelineMode m);
    PipelineMode getPipelineMode() const { return m_mode; }

    // Environment
    void setSkybox(AssetID id);
    void setAmbient(const Color& c);
    void setFog(bool en, const Color& c = Color::white(), f32 density = 0.01f);

    // Stats & accessors
    const RenderStats&  stats()    const { return m_stats; }
    RenderAPI           api()      const { return m_desc.api; }
    int                 width()    const { return m_width; }
    int                 height()   const { return m_height; }
    int                 activeRenderWidth() const { return m_sceneWidth > 0 ? m_sceneWidth : m_width; }
    int                 activeRenderHeight() const { return m_sceneHeight > 0 ? m_sceneHeight : m_height; }
    IRendererBackend&   backend()        { return *m_backend; }

private:
    void buildDefaultPasses();
    bool ensureFallbackResources();
    void destroyFallbackResources();
    void destroySceneViewport();
    void flushMeshQueue();
    void flushSpriteQueue();
    void runPostProcess();

    RenderPipelineDesc m_desc;
    int                m_width  = 1280;
    int                m_height = 720;
    PipelineMode       m_mode   = PipelineMode::PBR_3D;
    bool               m_inFrame = false;

    Scope<IRendererBackend> m_backend;
    std::vector<RenderPass> m_passes;
    std::vector<PostEffect> m_postEffects;
    std::vector<MeshDrawCmd>   m_meshQueue;
    std::vector<SpriteDrawCmd> m_spriteQueue;
    std::vector<LightCmd>      m_lights;

    AssetID m_skyboxID    = NULL_ASSET;
    Color   m_ambient     = {0.1f,0.1f,0.1f,1.0f};
    bool    m_fogEnabled  = false;
    Color   m_fogColor    = Color::white();
    f32     m_fogDensity  = 0.01f;
    f64     m_frameStartTime = 0.0;
    Mat4    m_viewProjection = Mat4::identity();
    Color   m_clearColor = Color::black();
    GpuHandle m_sceneFramebuffer = NULL_GPU_HANDLE;
    int       m_sceneWidth = 0;
    int       m_sceneHeight = 0;

    GpuHandle m_meshProgram    = NULL_GPU_HANDLE;
    GpuHandle m_meshVertex     = NULL_GPU_HANDLE;
    GpuHandle m_meshFragment   = NULL_GPU_HANDLE;
    GpuHandle m_meshVao        = NULL_GPU_HANDLE;
    GpuHandle m_meshVbo        = NULL_GPU_HANDLE;
    GpuHandle m_meshIbo        = NULL_GPU_HANDLE;
    int       m_meshIndexCount = 0;

    GpuHandle m_spriteProgram    = NULL_GPU_HANDLE;
    GpuHandle m_spriteVertex     = NULL_GPU_HANDLE;
    GpuHandle m_spriteFragment   = NULL_GPU_HANDLE;
    GpuHandle m_spriteVao        = NULL_GPU_HANDLE;
    GpuHandle m_spriteVbo        = NULL_GPU_HANDLE;
    GpuHandle m_spriteIbo        = NULL_GPU_HANDLE;
    GpuHandle m_whiteTexture     = NULL_GPU_HANDLE;
    int       m_spriteIndexCount = 0;

    RenderStats m_stats;
};

} // namespace Feliss
