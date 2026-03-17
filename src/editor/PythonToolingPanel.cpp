#include "editor/PythonToolingPanel.h"

#include "core/Engine.h"
#include "core/Logger.h"
#include "editor/PythonTooling.h"

#ifdef FELISS_HAS_IMGUI
#  include <imgui.h>
#endif

#include <string>

namespace Feliss::EditorPanels {

void DrawPythonToolingPanel(Engine& engine) {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Python Tooling")) {
        ImGui::End();
        return;
    }

    PythonTooling tooling(engine.config().projectPath + "/tools/python");
    const auto scripts = tooling.discoverScripts();
    const std::string interpreter = tooling.findInterpreter();

    ImGui::TextDisabled("%s", tooling.toolsRoot().c_str());
    ImGui::Text("Interpreter: %s", interpreter.empty() ? "not found" : interpreter.c_str());
    ImGui::Separator();

    for (const auto& script : scripts) {
        ImGui::TextUnformatted(script.name.c_str());
        ImGui::SameLine();
        const std::string runLabel = "Run##" + script.path;
        if (ImGui::SmallButton(runLabel.c_str())) {
            std::string error;
            const bool ok = tooling.runScript(script.path, {}, &error);
            if (ok) {
                FLS_INFOF("PythonTooling", "Ran Python tool: " << script.path);
            } else {
                FLS_ERRORF("PythonTooling", error);
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", script.path.c_str());
        }
    }

    if (scripts.empty()) {
        ImGui::TextDisabled("No Python tools found");
    }

    ImGui::End();
#else
    (void)engine;
#endif
}

} // namespace Feliss::EditorPanels
