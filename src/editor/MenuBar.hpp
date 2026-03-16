#pragma once

#include "core/FileSystem.h"
#include "core/Logger.h"
#include "core/Timer.h"

#include <functional>
#include <cstdio>
#include <string>

#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
#  include <imgui.h>
#endif

namespace Feliss {

struct MenuBarContext {
    bool*        showGrid = nullptr;
    std::string* configPath = nullptr;
    std::string* scenePath = nullptr;
    std::function<void()> newScene;
    std::function<void()> openScene;
    std::function<void()> saveScene;
    std::function<void()> saveSceneAs;
    std::function<void()> exportGame;
    std::function<void()> reloadScripts;
    std::function<void()> stopEditor;
    std::function<std::string()> buildConfigText;
    std::function<void()> triggerConfigReload;
    std::function<float()> queryFps;
};

#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
inline void DrawMenuBar(MenuBarContext& context) {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
            if (context.newScene) {
                context.newScene();
            }
        }
        if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
            if (context.openScene) {
                context.openScene();
            }
        }
        if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
            if (context.saveScene) {
                context.saveScene();
            }
        }
        if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
            if (context.saveSceneAs) {
                context.saveSceneAs();
            }
        }

        if (context.configPath && context.buildConfigText) {
            if (ImGui::MenuItem("Save Editor Config")) {
                const bool saved = FileSystem::WriteTextFile(*context.configPath, context.buildConfigText());
                FLS_INFOF("Editor", (saved ? "Saved" : "Failed to save") << " editor config: " << *context.configPath);
            }
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Alt+F4")) {
            if (context.stopEditor) {
                context.stopEditor();
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (context.showGrid) {
            ImGui::MenuItem("Grid", nullptr, context.showGrid);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Build")) {
        if (ImGui::MenuItem("Export Runtime Package")) {
            if (context.exportGame) {
                context.exportGame();
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Scripts")) {
        if (ImGui::MenuItem("Reload All")) {
            if (context.reloadScripts) {
                context.reloadScripts();
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Config")) {
        if (context.configPath) {
            ImGui::TextDisabled("%s", context.configPath->c_str());
            ImGui::Separator();
            const bool exists = FileSystem::Exists(*context.configPath);
            ImGui::Text("Exists: %s", exists ? "yes" : "no");
        }
        if (context.scenePath) {
            ImGui::Text("Scene: %s", context.scenePath->c_str());
        }
        if (ImGui::MenuItem("Reload Now") && context.triggerConfigReload) {
            context.triggerConfigReload();
        }
        ImGui::EndMenu();
    }

    char info[64];
    std::snprintf(info, sizeof(info), "%.1f fps", context.queryFps ? context.queryFps() : 0.0f);
    const float infoWidth = ImGui::CalcTextSize(info).x + 16.0f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - infoWidth);
    ImGui::TextDisabled("%s", info);

    ImGui::EndMainMenuBar();
}
#endif

} // namespace Feliss
