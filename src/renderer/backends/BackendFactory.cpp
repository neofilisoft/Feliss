#include "renderer/IRendererBackend.h"
#include "renderer/backends/OpenGLBackend.h"
#include "renderer/backends/VulkanBackend.h"
#include "renderer/backends/D3D11Backend.h"
#include "renderer/backends/D3D12Backend.h"
#include "renderer/backends/MetalBackend.h"
#include "core/Logger.h"

namespace Feliss {

Scope<IRendererBackend> createRendererBackend(RenderAPI api) {
    switch (api) {
#ifdef FELISS_RENDERER_OPENGL
        case RenderAPI::OpenGL:
            FLS_INFO("Renderer", "Creating OpenGL backend");
            return MakeScope<OpenGLBackend>();
#endif
#ifdef FELISS_RENDERER_VULKAN
        case RenderAPI::Vulkan:
            FLS_INFO("Renderer", "Creating Vulkan backend");
            return MakeScope<VulkanBackend>();
#endif
#ifdef FELISS_RENDERER_D3D11
        case RenderAPI::DirectX11:
            FLS_INFO("Renderer", "Creating DirectX 11 backend");
            return MakeScope<D3D11Backend>();
#endif
#ifdef FELISS_RENDERER_D3D12
        case RenderAPI::DirectX12:
            FLS_INFO("Renderer", "Creating DirectX 12 backend");
            return MakeScope<D3D12Backend>();
#endif
#ifdef FELISS_RENDERER_METAL
        case RenderAPI::Metal:
            FLS_INFO("Renderer", "Creating Metal backend");
            return MakeScope<MetalBackend>();
#endif
        default:
            FLS_WARNF("Renderer", "API " << RenderAPIToString(api)
                << " not compiled in - falling back to OpenGL");
            return MakeScope<OpenGLBackend>();
    }
}

bool D3D11Backend::init(Window&) {
    FLS_WARN("D3D11", "DirectX 11 backend is still a compatibility stub - prefer DirectX 12");
    return false;
}

void D3D11Backend::shutdown() {}

} // namespace Feliss
