#include "renderer/backends/VulkanBackend.h"

#include "core/Logger.h"
#include "renderer/ShaderCompiler.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace Feliss {

namespace {

bool hasRuntimeLibrary(const char* libraryName) {
#ifdef _WIN32
    HMODULE module = LoadLibraryA(libraryName);
    if (!module) {
        return false;
    }
    FreeLibrary(module);
    return true;
#else
    return true;
#endif
}

} // namespace

bool VulkanBackend::init(Window&) {
    ShaderCompiler compiler;
    const std::string glslc = compiler.findGLSLC();

    if (!hasRuntimeLibrary("vulkan-1.dll")) {
        FLS_ERROR("Vulkan", "Vulkan runtime missing: vulkan-1.dll not found");
        return false;
    }
    if (glslc.empty()) {
        FLS_WARN("Vulkan", "glslc.exe not found in PATH - SPIR-V precompile is unavailable");
    } else {
        FLS_INFOF("Vulkan", "Shader compiler detected: " << glslc);
    }

    FLS_WARN("Vulkan", "Vulkan explicit backend skeleton is wired, but swapchain/command buffer implementation is not finished yet");
    return false;
}

void VulkanBackend::shutdown() {}
void VulkanBackend::imguiInit() {}
void VulkanBackend::imguiNewFrame() {}
void VulkanBackend::imguiRender() {}
void VulkanBackend::imguiShutdown() {}

} // namespace Feliss
