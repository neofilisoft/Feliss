#include "editor/AssetBrowserPanel.h"

#include "assets/AssetRegistry.h"
#include "core/Engine.h"

#ifdef FELISS_HAS_IMGUI
#  include <imgui.h>
#endif

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace Feliss::EditorPanels {

#ifdef FELISS_HAS_IMGUI
namespace {

void drawDirectoryNode(const std::filesystem::path& directoryPath, std::string& currentDirectory) {
    std::error_code ec;
    std::vector<std::filesystem::path> childDirectories;
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath, ec)) {
        if (ec) {
            break;
        }
        if (entry.is_directory()) {
            childDirectories.push_back(entry.path());
        }
    }

    std::sort(childDirectories.begin(), childDirectories.end());
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (childDirectories.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (currentDirectory == directoryPath.generic_string()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    const bool open = ImGui::TreeNodeEx(directoryPath.generic_string().c_str(), flags, "%s",
        directoryPath.filename().empty() ? directoryPath.generic_string().c_str() : directoryPath.filename().string().c_str());
    if (ImGui::IsItemClicked()) {
        currentDirectory = directoryPath.generic_string();
    }

    if (!open) {
        return;
    }
    for (const auto& childDirectory : childDirectories) {
        drawDirectoryNode(childDirectory, currentDirectory);
    }
    ImGui::TreePop();
}

} // namespace
#endif

void DrawAssetBrowserPanel(Engine& engine, std::string& currentDirectory, float iconSize) {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Asset Browser")) {
        ImGui::End();
        return;
    }

    AssetRegistry& registry = engine.assets();
    const std::filesystem::path rootPath(registry.rootPath());
    if (currentDirectory.empty()) {
        currentDirectory = registry.rootPath();
    }

    if (ImGui::Button("Rescan Assets")) {
        registry.scan();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%s", registry.rootPath().c_str());
    ImGui::Separator();

    ImGui::BeginChild("AssetTree", ImVec2(220, 0), true);
    std::error_code ec;
    if (std::filesystem::is_directory(rootPath, ec)) {
        drawDirectoryNode(rootPath, currentDirectory);
    } else {
        ImGui::TextDisabled("Asset root missing");
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("AssetGrid", ImVec2(0, 0), false);

    const std::filesystem::path currentPath(currentDirectory);
    if (std::filesystem::is_directory(currentPath, ec)) {
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columns = (std::max)(1, static_cast<int>(panelWidth / (iconSize + 18.0f)));
        int columnIndex = 0;

        for (const auto& entry : std::filesystem::directory_iterator(currentPath, ec)) {
            if (ec || entry.path().extension() == ".meta") {
                continue;
            }

            const std::string fileName = entry.path().filename().string();
            const bool isDirectory = entry.is_directory();
            ImGui::BeginGroup();
            const std::string label = fileName.substr(0, (std::min)(static_cast<size_t>(12), fileName.size()))
                + "##" + entry.path().generic_string();
            if (ImGui::Button(label.c_str(), ImVec2(iconSize, iconSize))) {
                if (isDirectory) {
                    currentDirectory = entry.path().generic_string();
                }
            }
            if (!isDirectory) {
                if (const AssetRecord* record = registry.findByPath(entry.path().generic_string())) {
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("GUID: %s\nImporter: %s\nMeta: %s",
                            record->guid.toString().c_str(),
                            record->importer.c_str(),
                            record->metaPath.c_str());
                    }
                }
            }
            ImGui::TextWrapped("%s", fileName.c_str());
            ImGui::EndGroup();

            if (++columnIndex < columns) {
                ImGui::SameLine();
            } else {
                columnIndex = 0;
            }
        }
    } else {
        ImGui::TextDisabled("Folder missing: %s", currentDirectory.c_str());
    }

    ImGui::EndChild();
    ImGui::End();
#else
    (void)engine;
    (void)currentDirectory;
    (void)iconSize;
#endif
}

} // namespace Feliss::EditorPanels
