#include "core/Engine.h"
#include "core/Logger.h"
#include "editor/EditorSession.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

int main(int argc, char** argv) {
    Feliss::EngineConfig cfg;
    cfg.title        = "Feliss Editor";
    cfg.windowWidth  = 1600;
    cfg.windowHeight = 900;
    cfg.renderAPI    = Feliss::RenderAPI::OpenGL;
    cfg.mode         = Feliss::EngineMode::Editor;
    cfg.enableLua    = true;
    cfg.enableCSharp = false;
    cfg.projectPath  = ".";
    cfg.logFile      = "feliss_editor.log";

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--vulkan") cfg.renderAPI = Feliss::RenderAPI::Vulkan;
        else if (arg == "--dx11") cfg.renderAPI = Feliss::RenderAPI::DirectX11;
        else if (arg == "--dx12") cfg.renderAPI = Feliss::RenderAPI::DirectX12;
        else if (arg == "--metal") cfg.renderAPI = Feliss::RenderAPI::Metal;
        else if (arg == "--csharp") cfg.enableCSharp = true;
    }

    Feliss::Engine engine(cfg);
    if (!engine.init()) {
        FLS_FATAL("EditorMain", "Engine init failed");
#ifdef _WIN32
        MessageBoxA(nullptr,
            "Feliss Editor failed to initialize. Check feliss_editor.log for details.",
            "Feliss Editor Startup Error",
            MB_OK | MB_ICONERROR);
#endif
        return 1;
    }

    Feliss::EditorSession editor(engine);
    if (!editor.init()) {
        FLS_FATAL("EditorMain", "Editor session init failed");
        return 1;
    }

    engine.run();
    editor.shutdown();
    return 0;
}
