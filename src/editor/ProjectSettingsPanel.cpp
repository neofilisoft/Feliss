#include "editor/ProjectSettingsPanel.h"

#include "core/Engine.h"
#include "core/Logger.h"
#include "project/ProjectConfig.h"

#ifdef FELISS_HAS_IMGUI
#  include <imgui.h>
#endif

#include <cstdio>

namespace Feliss::EditorPanels {

void DrawProjectSettingsPanel(Engine& engine) {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Project Settings")) {
        ImGui::End();
        return;
    }

    ProjectConfig& config = engine.projectConfig();

    int scriptLanguage = static_cast<int>(config.defaultScriptLanguage);
    if (ImGui::Combo("Default Script Layer", &scriptLanguage, "CSharp\0Lua\0")) {
        config.defaultScriptLanguage = static_cast<ScriptLanguage>(scriptLanguage);
    }

    ImGui::Checkbox("Enable CSharp", &config.enableCSharp);
    ImGui::Checkbox("Enable Lua", &config.enableLua);

    int renderApi = static_cast<int>(config.preferredRenderAPI);
    if (ImGui::Combo("Preferred Render API", &renderApi, "None\0OpenGL\0Vulkan\0DirectX11\0DirectX12\0Metal\0WebGL\0")) {
        config.preferredRenderAPI = static_cast<RenderAPI>(renderApi);
    }

    int physicsBackend = static_cast<int>(config.physicsBackend);
    if (ImGui::Combo("Physics Backend", &physicsBackend, "None\0AsterCore\0PhysX\0Havok\0")) {
        config.physicsBackend = static_cast<PhysicsBackendType>(physicsBackend);
    }

    char sceneBuffer[260] = {};
    std::snprintf(sceneBuffer, sizeof(sceneBuffer), "%s", config.defaultScene.c_str());
    if (ImGui::InputText("Default Scene", sceneBuffer, sizeof(sceneBuffer))) {
        config.defaultScene = sceneBuffer;
    }

    char assetBuffer[260] = {};
    std::snprintf(assetBuffer, sizeof(assetBuffer), "%s", config.assetRoot.c_str());
    if (ImGui::InputText("Asset Root", assetBuffer, sizeof(assetBuffer))) {
        config.assetRoot = assetBuffer;
    }

    if (ImGui::Button("Save ProjectConfig")) {
        const bool saved = SaveProjectConfig(engine.projectConfigPath(), config);
        if (saved) {
            FLS_INFOF("ProjectSettings", "Saved project config: " << engine.projectConfigPath());
        } else {
            FLS_ERRORF("ProjectSettings", "Failed to save project config: " << engine.projectConfigPath());
        }
    }

    ImGui::End();
#else
    (void)engine;
#endif
}

} // namespace Feliss::EditorPanels
