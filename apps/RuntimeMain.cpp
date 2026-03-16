#include "core/Logger.h"
#include "runtime/RuntimeApplication.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

int main(int argc, char** argv) {
    Feliss::EngineConfig cfg;
    cfg.title        = "Feliss Runtime";
    cfg.windowWidth  = 1280;
    cfg.windowHeight = 720;
    cfg.renderAPI    = Feliss::RenderAPI::OpenGL;
    cfg.mode         = Feliss::EngineMode::Game;
    cfg.enableLua    = true;
    cfg.enableCSharp = false;
    cfg.projectPath  = ".";
    cfg.logFile      = "feliss_runtime.log";

    std::string bootScenePath = "./scene.fls";
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--scene" && i + 1 < argc) {
            bootScenePath = argv[++i];
        } else if (arg == "--vulkan") {
            cfg.renderAPI = Feliss::RenderAPI::Vulkan;
        } else if (arg == "--dx12") {
            cfg.renderAPI = Feliss::RenderAPI::DirectX12;
        } else if (arg == "--metal") {
            cfg.renderAPI = Feliss::RenderAPI::Metal;
        }
    }

    Feliss::RuntimeApplication app(cfg);
    if (!app.init(bootScenePath)) {
        FLS_FATAL("RuntimeMain", "Runtime init failed");
#ifdef _WIN32
        MessageBoxA(nullptr,
            "Feliss Runtime failed to initialize. Check feliss_runtime.log for details.",
            "Feliss Runtime Startup Error",
            MB_OK | MB_ICONERROR);
#endif
        return 1;
    }

    return app.run();
}
