#include "core/Engine.h"
#include "core/Logger.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#ifdef FELISS_EDITOR_BUILD
#  include "editor/EditorApp.h"
#endif

int main(int argc, char** argv) {
    Feliss::EngineConfig cfg;
    cfg.title        = "Feliss Engine";
    cfg.windowWidth  = 1280;
    cfg.windowHeight = 720;
    cfg.renderAPI    = Feliss::RenderAPI::OpenGL;
    cfg.mode         = Feliss::EngineMode::Editor;
    cfg.enableLua    = true;
    cfg.enableCSharp = false;
    cfg.projectPath  = ".";
    cfg.logFile      = "feliss.log";

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--game") {
            cfg.mode = Feliss::EngineMode::Game;
        } else if (arg == "--server") {
            cfg.mode = Feliss::EngineMode::Server;
        } else if (arg == "--vulkan") {
            cfg.renderAPI = Feliss::RenderAPI::Vulkan;
        } else if (arg == "--dx11") {
            cfg.renderAPI = Feliss::RenderAPI::DirectX11;
        } else if (arg == "--dx12") {
            cfg.renderAPI = Feliss::RenderAPI::DirectX12;
        } else if (arg == "--metal") {
            cfg.renderAPI = Feliss::RenderAPI::Metal;
        } else if (arg == "--csharp") {
            cfg.enableCSharp = true;
        }
    }

    Feliss::Engine engine(cfg);
    if (!engine.init()) {
        FLS_FATAL("Main", "Engine init failed");
#ifdef _WIN32
        MessageBoxA(nullptr,
            "Feliss failed to initialize. Check feliss.log for details.",
            "Feliss Startup Error",
            MB_OK | MB_ICONERROR);
#endif
        return 1;
    }

#ifdef FELISS_EDITOR_BUILD
    Feliss::EditorApp editor(engine);
    editor.init();
    engine.setRenderCallback([&] { editor.render(); });
#endif

    engine.run();
    return 0;
}
