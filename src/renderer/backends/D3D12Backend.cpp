#include "renderer/backends/D3D12Backend.h"

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
    return false;
#endif
}

} // namespace

bool D3D12Backend::init(Window&) {
    ShaderCompiler compiler;
    const std::string dxc = compiler.findDXC();

#ifdef _WIN32
    if (!hasRuntimeLibrary("d3d12.dll") || !hasRuntimeLibrary("dxgi.dll")) {
        FLS_ERROR("D3D12", "DirectX 12 runtime missing: d3d12.dll or dxgi.dll unavailable");
        return false;
    }
#else
    FLS_ERROR("D3D12", "DirectX 12 backend is Windows-only");
    return false;
#endif

    if (dxc.empty()) {
        FLS_WARN("D3D12", "dxc.exe not found in PATH - DXIL shader precompile is unavailable");
    } else {
        FLS_INFOF("D3D12", "Shader compiler detected: " << dxc);
    }

    FLS_WARN("D3D12", "DirectX 12 explicit backend skeleton is wired, but device/descriptor queue implementation is not finished yet");
    return false;
}

void D3D12Backend::shutdown() {}

} // namespace Feliss
