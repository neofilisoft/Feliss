#include "renderer/RenderPipeline.h"

#include "core/Logger.h"
#include "core/Timer.h"

#include <algorithm>
#include <array>

namespace Feliss {

namespace {

constexpr u32 kTriangles = 0x0004;

Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 out {};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            out.m[row][col] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                out.m[row][col] += a.m[row][k] * b.m[k][col];
            }
        }
    }
    return out;
}

bool isPassEnabled(const std::vector<RenderPass>& passes, RenderPassType type) {
    for (const auto& pass : passes) {
        if (pass.type == type) {
            return pass.enabled;
        }
    }
    return false;
}

Color fallbackMeshColor(const MeshDrawCmd& cmd, PipelineMode mode) {
    const f32 seed = static_cast<f32>((cmd.materialID ^ (static_cast<u64>(cmd.layer) << 8)) & 0xFFu) / 255.0f;
    switch (mode) {
        case PipelineMode::HD2D:
            return {0.35f + seed * 0.2f, 0.55f, 0.85f, 1.0f};
        case PipelineMode::Pixel2D:
            return {0.85f, 0.80f - seed * 0.2f, 0.25f + seed * 0.15f, 1.0f};
        case PipelineMode::Toon:
            return {0.20f + seed * 0.1f, 0.75f, 0.35f, 1.0f};
        case PipelineMode::Custom:
            return {0.70f, 0.35f + seed * 0.3f, 0.70f, 1.0f};
        case PipelineMode::PBR_3D:
        default:
            return {0.65f + seed * 0.2f, 0.68f + seed * 0.1f, 0.75f + seed * 0.05f, 1.0f};
    }
}

} // namespace

RenderPipeline::RenderPipeline(const RenderPipelineDesc& desc)
    : m_desc(desc), m_width(desc.width), m_height(desc.height) {
    m_backend = createRendererBackend(desc.api);
}

RenderPipeline::~RenderPipeline() {
    shutdown();
}

bool RenderPipeline::init(Window& window) {
    if (!m_backend) {
        FLS_ERROR("RenderPipeline", "No backend created");
        return false;
    }
    if (!m_backend->init(window)) {
        FLS_ERROR("RenderPipeline", "Backend init failed");
        return false;
    }
    m_backend->setVSync(m_desc.vsync);
    buildDefaultPasses();
    ensureFallbackResources();
    FLS_INFOF("RenderPipeline", "RenderPipeline initialized ["
        << m_backend->apiName() << "] "
        << m_width << "x" << m_height);
    return true;
}

void RenderPipeline::shutdown() {
    destroySceneViewport();
    destroyFallbackResources();
    if (m_backend) {
        m_backend->shutdown();
    }
}

void RenderPipeline::onResize(int w, int h) {
    m_width = w;
    m_height = h;
    if (m_backend) {
        m_backend->onResize(w, h);
    }
}

void RenderPipeline::beginFrame() {
    if (!m_backend || m_inFrame) {
        return;
    }
    m_inFrame = true;
    m_stats = {};
    m_meshQueue.clear();
    m_spriteQueue.clear();
    m_lights.clear();
    m_frameStartTime = Timer::GetTimeSeconds();
    m_backend->setClearColor(m_clearColor);
    m_backend->beginFrame();
}

void RenderPipeline::endFrame() {
    renderScenePass();
    presentFrame();
}

void RenderPipeline::renderScenePass() {
    if (!m_backend || !m_inFrame) {
        return;
    }

    if (m_sceneFramebuffer != NULL_GPU_HANDLE && m_sceneWidth > 0 && m_sceneHeight > 0) {
        m_backend->bindFramebuffer(m_sceneFramebuffer);
        m_backend->setViewport({0.0f, 0.0f, static_cast<f32>(m_sceneWidth), static_cast<f32>(m_sceneHeight), 0.0f, 1.0f});
        m_backend->setClearColor(m_clearColor);
        m_backend->clear(true, true, false);
    }

    flushMeshQueue();
    flushSpriteQueue();
    runPostProcess();

    if (m_sceneFramebuffer != NULL_GPU_HANDLE) {
        m_backend->bindFramebuffer(NULL_GPU_HANDLE);
        m_backend->setViewport({0.0f, 0.0f, static_cast<f32>(m_width), static_cast<f32>(m_height), 0.0f, 1.0f});
    }
}

void RenderPipeline::presentFrame() {
    if (!m_backend || !m_inFrame) {
        return;
    }

    m_backend->endFrame();
    m_backend->present();
    m_stats.cpuMs = static_cast<f32>((Timer::GetTimeSeconds() - m_frameStartTime) * 1000.0);
    m_inFrame = false;
}

void RenderPipeline::beginImGui() {
    if (m_backend) {
        m_backend->imguiNewFrame();
    }
}

void RenderPipeline::endImGui() {
    if (m_backend) {
        m_backend->imguiRender();
    }
}

void RenderPipeline::submitMesh(AssetID mesh, AssetID mat,
                                const Mat4& transform,
                                bool castShadow, u32 layer) {
    MeshDrawCmd cmd;
    cmd.meshID = mesh;
    cmd.materialID = mat;
    cmd.transform = transform;
    cmd.castShadow = castShadow;
    cmd.layer = layer;
    m_meshQueue.push_back(cmd);
}

void RenderPipeline::submitSprite(AssetID tex, const Color& tint,
                                  const Vec2& size, const Mat4& transform,
                                  int sortOrder) {
    SpriteDrawCmd cmd;
    cmd.texID = tex;
    cmd.tint = tint;
    cmd.size = size;
    cmd.transform = transform;
    cmd.sortOrder = sortOrder;
    m_spriteQueue.push_back(cmd);
}

void RenderPipeline::submitLight(int lightType, const Color& color,
                                 f32 intensity, const Vec3& pos,
                                 const Vec3& dir, f32 range) {
    LightCmd light;
    light.lightType = lightType;
    light.color = color;
    light.intensity = intensity;
    light.position = pos;
    light.direction = dir;
    light.range = range;
    m_lights.push_back(light);
    m_stats.lightCount = static_cast<u32>(m_lights.size());
}

void RenderPipeline::setCamera(const Mat4& viewProjection, const Color& clearColor) {
    m_viewProjection = viewProjection;
    m_clearColor = clearColor;
    if (m_backend) {
        m_backend->setClearColor(clearColor);
    }
}

void RenderPipeline::buildDefaultPasses() {
    m_passes.clear();

    RenderPass shadow;
    shadow.name = "ShadowMap";
    shadow.type = RenderPassType::ShadowMap;
    shadow.enabled = true;
    m_passes.push_back(shadow);

    RenderPass opaque;
    opaque.name = "Opaque";
    opaque.type = RenderPassType::Opaque;
    opaque.enabled = true;
    m_passes.push_back(opaque);

    RenderPass skybox;
    skybox.name = "Skybox";
    skybox.type = RenderPassType::Skybox;
    skybox.enabled = true;
    m_passes.push_back(skybox);

    RenderPass transparent;
    transparent.name = "Transparent";
    transparent.type = RenderPassType::Transparent;
    transparent.enabled = true;
    m_passes.push_back(transparent);

    RenderPass pp;
    pp.name = "PostProcess";
    pp.type = RenderPassType::PostProcess;
    pp.enabled = true;
    m_passes.push_back(pp);

    RenderPass ui;
    ui.name = "UI";
    ui.type = RenderPassType::UI;
    ui.enabled = true;
    m_passes.push_back(ui);
}

void RenderPipeline::addPass(const RenderPass& pass) {
    m_passes.push_back(pass);
}

void RenderPipeline::removePass(const std::string& name) {
    m_passes.erase(std::remove_if(m_passes.begin(), m_passes.end(),
        [&](const RenderPass& pass) { return pass.name == name; }), m_passes.end());
}

RenderPass* RenderPipeline::getPass(const std::string& name) {
    for (auto& pass : m_passes) {
        if (pass.name == name) {
            return &pass;
        }
    }
    return nullptr;
}

void RenderPipeline::setPassEnabled(const std::string& name, bool en) {
    if (auto* pass = getPass(name)) {
        pass->enabled = en;
    }
}

void RenderPipeline::addPostEffect(const PostEffect& fx) {
    m_postEffects.push_back(fx);
}

void RenderPipeline::removePostEffect(const std::string& name) {
    m_postEffects.erase(std::remove_if(m_postEffects.begin(), m_postEffects.end(),
        [&](const PostEffect& effect) { return effect.name == name; }), m_postEffects.end());
}

void RenderPipeline::setPostEffectEnabled(const std::string& name, bool en) {
    for (auto& effect : m_postEffects) {
        if (effect.name == name) {
            effect.enabled = en;
            return;
        }
    }
}

GpuHandle RenderPipeline::createRenderTarget(int w, int h, PixelFormat fmt, bool hasDepth) {
    if (!m_backend) {
        return NULL_GPU_HANDLE;
    }
    return m_backend->createFramebuffer(w, h, fmt, hasDepth);
}

void RenderPipeline::destroyRenderTarget(GpuHandle h) {
    if (m_backend) {
        m_backend->destroyFramebuffer(h);
    }
}

GpuHandle RenderPipeline::getRenderTargetColorTex(GpuHandle h) {
    if (!m_backend) {
        return NULL_GPU_HANDLE;
    }
    return m_backend->framebufferColorTexture(h);
}

void RenderPipeline::setSceneViewportSize(int w, int h) {
    if (!m_backend) {
        return;
    }

    if (w <= 0 || h <= 0) {
        destroySceneViewport();
        return;
    }

    if (m_sceneFramebuffer != NULL_GPU_HANDLE && m_sceneWidth == w && m_sceneHeight == h) {
        return;
    }

    destroySceneViewport();
    m_sceneFramebuffer = createRenderTarget(w, h, PixelFormat::RGBA8, true);
    if (m_sceneFramebuffer == NULL_GPU_HANDLE) {
        FLS_ERRORF("RenderPipeline", "Failed to create scene viewport framebuffer " << w << "x" << h);
        return;
    }

    m_sceneWidth = w;
    m_sceneHeight = h;
    FLS_INFOF("RenderPipeline", "Scene viewport framebuffer ready: " << m_sceneWidth << "x" << m_sceneHeight);
}

GpuHandle RenderPipeline::sceneViewportTexture() const {
    if (!m_backend || m_sceneFramebuffer == NULL_GPU_HANDLE) {
        return NULL_GPU_HANDLE;
    }
    return m_backend->framebufferColorTexture(m_sceneFramebuffer);
}

void RenderPipeline::setPipelineMode(PipelineMode m) {
    m_mode = m;
    FLS_INFOF("RenderPipeline", "Pipeline mode: " << PipelineModeToString(m));
}

void RenderPipeline::setSkybox(AssetID id) {
    m_skyboxID = id;
}

void RenderPipeline::setAmbient(const Color& c) {
    m_ambient = c;
}

void RenderPipeline::setFog(bool en, const Color& c, f32 density) {
    m_fogEnabled = en;
    m_fogColor = c;
    m_fogDensity = density;
}

void RenderPipeline::destroySceneViewport() {
    if (!m_backend || m_sceneFramebuffer == NULL_GPU_HANDLE) {
        m_sceneFramebuffer = NULL_GPU_HANDLE;
        m_sceneWidth = 0;
        m_sceneHeight = 0;
        return;
    }

    m_backend->destroyFramebuffer(m_sceneFramebuffer);
    m_sceneFramebuffer = NULL_GPU_HANDLE;
    m_sceneWidth = 0;
    m_sceneHeight = 0;
}

bool RenderPipeline::ensureFallbackResources() {
    if (!m_backend) {
        return false;
    }
    if (m_meshProgram != NULL_GPU_HANDLE && m_spriteProgram != NULL_GPU_HANDLE) {
        return true;
    }

    static constexpr const char* kMeshVertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPosition;
        uniform mat4 uModel;
        uniform mat4 uViewProj;
        void main() {
            gl_Position = uViewProj * uModel * vec4(aPosition, 1.0);
        }
    )";

    static constexpr const char* kMeshFragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 uColor;
        void main() {
            FragColor = uColor;
        }
    )";

    static constexpr const char* kSpriteVertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 vTexCoord;
        uniform mat4 uModel;
        uniform mat4 uViewProj;
        uniform vec2 uSize;
        void main() {
            vec3 scaled = vec3(aPosition.xy * uSize, aPosition.z);
            gl_Position = uViewProj * uModel * vec4(scaled, 1.0);
            vTexCoord = aTexCoord;
        }
    )";

    static constexpr const char* kSpriteFragmentSource = R"(
        #version 330 core
        in vec2 vTexCoord;
        out vec4 FragColor;
        uniform sampler2D uTexture;
        uniform vec4 uTint;
        void main() {
            FragColor = texture(uTexture, vTexCoord) * uTint;
        }
    )";

    m_meshVertex = m_backend->createShader(ShaderStage::Vertex, kMeshVertexSource, true);
    m_meshFragment = m_backend->createShader(ShaderStage::Fragment, kMeshFragmentSource, true);
    m_meshProgram = m_backend->createProgram(m_meshVertex, m_meshFragment);

    m_spriteVertex = m_backend->createShader(ShaderStage::Vertex, kSpriteVertexSource, true);
    m_spriteFragment = m_backend->createShader(ShaderStage::Fragment, kSpriteFragmentSource, true);
    m_spriteProgram = m_backend->createProgram(m_spriteVertex, m_spriteFragment);

    if (m_meshProgram == NULL_GPU_HANDLE || m_spriteProgram == NULL_GPU_HANDLE) {
        FLS_WARN("RenderPipeline", "Fallback render resources unavailable on this backend");
        return false;
    }

    static constexpr std::array<float, 9> kTriangleVertices = {
         0.0f,  0.6f, 0.0f,
        -0.6f, -0.45f, 0.0f,
         0.6f, -0.45f, 0.0f
    };
    static constexpr std::array<u32, 3> kTriangleIndices = {
        0, 1, 2
    };

    static constexpr std::array<float, 20> kSpriteVertices = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };
    static constexpr std::array<u32, 6> kSpriteIndices = {0, 1, 2, 2, 3, 0};

    m_meshVao = m_backend->createVertexArray();
    m_backend->bindVertexArray(m_meshVao);
    m_meshVbo = m_backend->createVertexBuffer(kTriangleVertices.data(), sizeof(kTriangleVertices), BufferUsage::Static);
    m_meshIbo = m_backend->createIndexBuffer(kTriangleIndices.data(), sizeof(kTriangleIndices), BufferUsage::Static);
    m_backend->bindVertexBuffer(m_meshVbo);
    m_backend->bindIndexBuffer(m_meshIbo);
    m_backend->setVertexAttrib(0, 3, false, static_cast<int>(sizeof(float) * 3), 0);
    m_meshIndexCount = static_cast<int>(kTriangleIndices.size());

    m_spriteVao = m_backend->createVertexArray();
    m_backend->bindVertexArray(m_spriteVao);
    m_spriteVbo = m_backend->createVertexBuffer(kSpriteVertices.data(), sizeof(kSpriteVertices), BufferUsage::Static);
    m_spriteIbo = m_backend->createIndexBuffer(kSpriteIndices.data(), sizeof(kSpriteIndices), BufferUsage::Static);
    m_backend->bindVertexBuffer(m_spriteVbo);
    m_backend->bindIndexBuffer(m_spriteIbo);
    m_backend->setVertexAttrib(0, 3, false, static_cast<int>(sizeof(float) * 5), 0);
    m_backend->setVertexAttrib(1, 2, false, static_cast<int>(sizeof(float) * 5), sizeof(float) * 3);
    m_spriteIndexCount = static_cast<int>(kSpriteIndices.size());

    const std::array<unsigned char, 4> whitePixel = {255, 255, 255, 255};
    m_whiteTexture = m_backend->createTexture2D(1, 1, PixelFormat::RGBA8, whitePixel.data(),
        TextureFilter::Nearest, TextureWrap::Clamp);

    FLS_INFO("RenderPipeline", "Created fallback mesh/sprite render resources");
    return true;
}

void RenderPipeline::destroyFallbackResources() {
    if (!m_backend) {
        return;
    }

    if (m_whiteTexture != NULL_GPU_HANDLE) {
        m_backend->destroyTexture(m_whiteTexture);
        m_whiteTexture = NULL_GPU_HANDLE;
    }

    if (m_meshIbo != NULL_GPU_HANDLE) {
        m_backend->destroyBuffer(m_meshIbo);
        m_meshIbo = NULL_GPU_HANDLE;
    }
    if (m_meshVbo != NULL_GPU_HANDLE) {
        m_backend->destroyBuffer(m_meshVbo);
        m_meshVbo = NULL_GPU_HANDLE;
    }
    if (m_meshVao != NULL_GPU_HANDLE) {
        m_backend->destroyVertexArray(m_meshVao);
        m_meshVao = NULL_GPU_HANDLE;
    }

    if (m_spriteIbo != NULL_GPU_HANDLE) {
        m_backend->destroyBuffer(m_spriteIbo);
        m_spriteIbo = NULL_GPU_HANDLE;
    }
    if (m_spriteVbo != NULL_GPU_HANDLE) {
        m_backend->destroyBuffer(m_spriteVbo);
        m_spriteVbo = NULL_GPU_HANDLE;
    }
    if (m_spriteVao != NULL_GPU_HANDLE) {
        m_backend->destroyVertexArray(m_spriteVao);
        m_spriteVao = NULL_GPU_HANDLE;
    }

    if (m_meshProgram != NULL_GPU_HANDLE) {
        m_backend->destroyProgram(m_meshProgram);
        m_meshProgram = NULL_GPU_HANDLE;
    }
    if (m_meshVertex != NULL_GPU_HANDLE) {
        m_backend->destroyShader(m_meshVertex);
        m_meshVertex = NULL_GPU_HANDLE;
    }
    if (m_meshFragment != NULL_GPU_HANDLE) {
        m_backend->destroyShader(m_meshFragment);
        m_meshFragment = NULL_GPU_HANDLE;
    }

    if (m_spriteProgram != NULL_GPU_HANDLE) {
        m_backend->destroyProgram(m_spriteProgram);
        m_spriteProgram = NULL_GPU_HANDLE;
    }
    if (m_spriteVertex != NULL_GPU_HANDLE) {
        m_backend->destroyShader(m_spriteVertex);
        m_spriteVertex = NULL_GPU_HANDLE;
    }
    if (m_spriteFragment != NULL_GPU_HANDLE) {
        m_backend->destroyShader(m_spriteFragment);
        m_spriteFragment = NULL_GPU_HANDLE;
    }
}

void RenderPipeline::flushMeshQueue() {
    if (m_meshQueue.empty() || !m_backend || !isPassEnabled(m_passes, RenderPassType::Opaque)) {
        return;
    }
    if (!ensureFallbackResources() || m_meshProgram == NULL_GPU_HANDLE || m_meshVao == NULL_GPU_HANDLE) {
        return;
    }

    std::sort(m_meshQueue.begin(), m_meshQueue.end(),
        [](const MeshDrawCmd& a, const MeshDrawCmd& b) {
            if (a.layer != b.layer) {
                return a.layer < b.layer;
            }
            return a.materialID < b.materialID;
        });

    m_backend->setDepthTest(true);
    m_backend->setDepthWrite(true);
    m_backend->setCullMode(CullMode::Back);
    m_backend->setBlendMode(BlendMode::Opaque);
    m_backend->bindProgram(m_meshProgram);
    m_backend->bindVertexArray(m_meshVao);
    m_backend->bindIndexBuffer(m_meshIbo);

    for (const auto& cmd : m_meshQueue) {
        const Color tint = fallbackMeshColor(cmd, m_mode);
        m_backend->setUniformM4(m_meshProgram, "uModel", cmd.transform);
        m_backend->setUniformM4(m_meshProgram, "uViewProj", m_viewProjection);
        m_backend->setUniformV4(m_meshProgram, "uColor", {tint.r, tint.g, tint.b, tint.a});
        m_backend->drawIndexed(kTriangles, m_meshIndexCount, true);
        ++m_stats.drawCalls;
        m_stats.triangles += static_cast<u32>(m_meshIndexCount / 3);
    }
}

void RenderPipeline::flushSpriteQueue() {
    if (m_spriteQueue.empty() || !m_backend) {
        return;
    }
    if (!isPassEnabled(m_passes, RenderPassType::Transparent) && !isPassEnabled(m_passes, RenderPassType::UI)) {
        return;
    }
    if (!ensureFallbackResources() || m_spriteProgram == NULL_GPU_HANDLE || m_spriteVao == NULL_GPU_HANDLE) {
        return;
    }

    std::sort(m_spriteQueue.begin(), m_spriteQueue.end(),
        [](const SpriteDrawCmd& a, const SpriteDrawCmd& b) {
            if (a.sortOrder != b.sortOrder) {
                return a.sortOrder < b.sortOrder;
            }
            return a.texID < b.texID;
        });

    m_backend->setDepthTest(false);
    m_backend->setDepthWrite(false);
    m_backend->setCullMode(CullMode::None);
    m_backend->setBlendMode(BlendMode::Alpha);
    m_backend->bindProgram(m_spriteProgram);
    m_backend->bindVertexArray(m_spriteVao);
    m_backend->bindIndexBuffer(m_spriteIbo);
    m_backend->setUniformI(m_spriteProgram, "uTexture", 0);

    AssetID lastTexture = static_cast<AssetID>(-1);
    for (const auto& cmd : m_spriteQueue) {
        const GpuHandle textureHandle = cmd.texID != NULL_ASSET
            ? static_cast<GpuHandle>(cmd.texID)
            : m_whiteTexture;
        if (cmd.texID != lastTexture) {
            m_backend->bindTexture(textureHandle, 0);
            lastTexture = cmd.texID;
            ++m_stats.textureBinds;
        }
        m_backend->setUniformM4(m_spriteProgram, "uModel", cmd.transform);
        m_backend->setUniformM4(m_spriteProgram, "uViewProj", m_viewProjection);
        m_backend->setUniformV2(m_spriteProgram, "uSize", cmd.size);
        m_backend->setUniformV4(m_spriteProgram, "uTint", {cmd.tint.r, cmd.tint.g, cmd.tint.b, cmd.tint.a});
        m_backend->drawIndexed(kTriangles, m_spriteIndexCount, true);
        ++m_stats.drawCalls;
        ++m_stats.spriteCalls;
        m_stats.triangles += static_cast<u32>(m_spriteIndexCount / 3);
    }
}

void RenderPipeline::runPostProcess() {
    if (!m_backend || !isPassEnabled(m_passes, RenderPassType::PostProcess)) {
        return;
    }
    for (auto& effect : m_postEffects) {
        if (!effect.enabled || effect.shaderID == NULL_ASSET) {
            continue;
        }
        FLS_DEBUG("RenderPipeline", "Post-process effect queued but custom fullscreen path is not wired yet");
    }
}

} // namespace Feliss
