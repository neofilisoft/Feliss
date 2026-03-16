#include "editor/EditorApp.h"
#include "editor/AssetBrowserPanel.h"
#include "editor/FileDialogs.h"
#include "editor/InspectorPanel.h"
#include "editor/MenuBar.hpp"
#include "editor/ProjectExporter.h"
#include "core/Engine.h"
#include "core/FileSystem.h"
#include "core/Timer.h"
#include "ecs/World.h"
#include "ecs/Component.h"
#include "scripting/ScriptEngine.h"
#include "renderer/RenderPipeline.h"
#include "core/Logger.h"

#ifdef FELISS_EDITOR_BUILD
#  ifdef FELISS_HAS_IMGUI
#    include <imgui.h>
#    include <imgui_internal.h>
#  endif
#endif

#include <cstring>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <sstream>

namespace Feliss {

EditorApp::EditorApp(Engine& e) : m_engine(e) {}

bool EditorApp::init() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    ImGuiIO& io = ImGui::GetIO();
#ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
    (void)io;
    ImGui::StyleColorsDark();
    m_assetRootPath = std::filesystem::path(m_engine.config().projectPath + "/assets").lexically_normal().generic_string();
    m_assetViewPath = m_assetRootPath;
    m_scenePath = std::filesystem::path(m_engine.config().projectPath + "/scene.fls").lexically_normal().generic_string();
    m_editorConfigPath = std::filesystem::path(m_engine.config().projectPath + "/editor.cfg").lexically_normal().generic_string();
    m_engine.assets().setRootPath(m_assetRootPath);
    m_engine.assets().scan();
    createDefaultScene();
    ensureEditorConfigFile();
    m_configHotReload.start(m_editorConfigPath, [this](const std::string&, const std::string& contents) {
        applyEditorConfig(contents, true);
    });
    triggerConfigReload();
    FLS_INFO("Editor","EditorApp initialized");
    return true;
#else
    return true;
#endif
}

void EditorApp::shutdown() {}

void EditorApp::render() {
#if defined(FELISS_EDITOR_BUILD) && defined(FELISS_HAS_IMGUI)
    m_configHotReload.poll();
    drawMainMenuBar();
    drawDockspace();
    drawToolbar();
    drawHierarchy();
    drawInspector();
    drawSceneView();
    drawAssetBrowser();
    drawConsole();
    drawRenderPipelineEditor();
#endif
}

// ---- Main menu bar ----
void EditorApp::drawMainMenuBar() {
#ifdef FELISS_HAS_IMGUI
    MenuBarContext context;
    context.showGrid = &m_showGrid;
    context.configPath = &m_editorConfigPath;
    context.scenePath = &m_scenePath;
    context.newScene = [this]() {
        newScene();
    };
    context.openScene = [this]() {
        openScene();
    };
    context.saveScene = [this]() {
        saveScene(false);
    };
    context.saveSceneAs = [this]() {
        saveScene(true);
    };
    context.exportGame = [this]() {
        exportRuntimePackage();
    };
    context.reloadScripts = [this]() {
        m_engine.scripts().reloadAll();
    };
    context.stopEditor = [this]() {
        m_engine.stop();
    };
    context.buildConfigText = [this]() { return buildEditorConfigText(); };
    context.triggerConfigReload = [this]() { triggerConfigReload(); };
    context.queryFps = [this]() { return m_engine.fps(); };
    DrawMenuBar(context);
#endif
}

// ---- Dockspace ----
void EditorApp::drawDockspace() {
#ifdef FELISS_HAS_IMGUI
#ifdef IMGUI_HAS_DOCK
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("##DockRoot", nullptr, flags);
    ImGui::PopStyleVar();
    ImGui::DockSpace(ImGui::GetID("MainDock"),
        ImVec2(0,0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
#endif
#endif
}

// ---- Toolbar ----
void EditorApp::drawToolbar() {
#ifdef FELISS_HAS_IMGUI
    ImGui::Begin("##Toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove);

    // Play / Pause / Stop
    if (m_playState == PlayState::Stopped) {
        if (ImGui::Button("  Play  ")) {
            m_playState = PlayState::Playing;
            FLS_INFO("Editor","Play");
        }
    } else {
        if (ImGui::Button("  Stop  ")) {
            m_playState = PlayState::Stopped;
            FLS_INFO("Editor","Stop");
        }
        ImGui::SameLine();
        const char* pauseLabel = (m_playState==PlayState::Paused) ? "Resume" : " Pause ";
        if (ImGui::Button(pauseLabel))
            m_playState = (m_playState==PlayState::Playing) ? PlayState::Paused : PlayState::Playing;
    }

    ImGui::SameLine(0, 20);

    // Gizmo tools
    const char* tools[] = {"Select","Move","Rotate","Scale"};
    for (int i = 0; i < 4; ++i) {
        bool active = (m_tool == static_cast<GizmoTool>(i));
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        if (ImGui::Button(tools[i])) m_tool = static_cast<GizmoTool>(i);
        if (active) ImGui::PopStyleColor();
        ImGui::SameLine();
    }
    ImGui::End();
#endif
}

// ---- Hierarchy ----
void EditorApp::drawHierarchy() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Hierarchy")) { ImGui::End(); return; }

    if (ImGui::Button("+ Entity")) {
        m_selected = m_engine.world().createEntity("Entity");
    }

    ImGui::Separator();

    auto roots = m_engine.world().getRoots();
    for (EntityID id : roots) drawEntityNode(id);

    // Deselect on blank click
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
        m_selected = NULL_ENTITY;

    ImGui::End();
#endif
}

void EditorApp::drawEntityNode(EntityID id) {
#ifdef FELISS_HAS_IMGUI
    World& w = m_engine.world();
    std::string name = w.getName(id);
    bool hasChildren = !w.getChildren(id).empty();

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!hasChildren)  flags |= ImGuiTreeNodeFlags_Leaf;
    if (id == m_selected) flags |= ImGuiTreeNodeFlags_Selected;

    // Inline rename
    bool open = false;
    if (m_renaming && m_renamingID == id) {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##rename", m_renameBuffer, sizeof(m_renameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            w.setName(id, m_renameBuffer);
            m_renaming = false;
        }
        if (!ImGui::IsItemActive() && !ImGui::IsItemFocused()) m_renaming = false;
    } else {
        open = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "%s", name.c_str());
        if (ImGui::IsItemClicked()) {
            m_selected = id;
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            m_renaming = true;
            m_renamingID = id;
            std::strncpy(m_renameBuffer, name.c_str(), 255);
            m_renameBuffer[255] = '\0';
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Create Child")) {
                EntityID child = w.createEntity("Child");
                w.setParent(child, id);
            }
            if (ImGui::MenuItem("Duplicate")) {
                EntityID dup = w.createEntity(name + "_copy");
                w.setParent(dup, w.getParent(id));
                m_selected = dup;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete")) {
                w.destroyEntity(id);
                if (m_selected == id) m_selected = NULL_ENTITY;
                ImGui::EndPopup();
                if (open) ImGui::TreePop();
                return;
            }
            ImGui::EndPopup();
        }
    }

    if (open) {
        for (EntityID child : w.getChildren(id)) drawEntityNode(child);
        ImGui::TreePop();
    }
#endif
}

// ---- Inspector ----
void EditorApp::drawInspector() {
#ifdef FELISS_HAS_IMGUI
    EditorPanels::DrawInspectorPanel(m_engine, m_selected);
#endif
}

// ---- Scene View ----
void EditorApp::drawSceneView() {
#ifdef FELISS_HAS_IMGUI
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    if (!ImGui::Begin("Scene View")) { ImGui::PopStyleVar(); ImGui::End(); return; }
    ImVec2 sz = ImGui::GetContentRegionAvail();
    if (sz.x > 1 && sz.y > 1) {
        auto& renderer = m_engine.renderer();
        renderer.setSceneViewportSize(static_cast<int>(sz.x), static_cast<int>(sz.y));
        const GpuHandle sceneTexture = renderer.sceneViewportTexture();
        if (sceneTexture != NULL_GPU_HANDLE) {
            const ImTextureID image = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(sceneTexture));
            ImGui::Image(image, sz, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::SetCursorPos(ImVec2(12, 28));
            ImGui::TextDisabled("Scene %dx%d | %s",
                (int)sz.x, (int)sz.y,
                PipelineModeToString(renderer.getPipelineMode()));
        } else {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImVec2 min = ImGui::GetCursorScreenPos();
            const ImVec2 max = ImVec2(min.x + sz.x, min.y + sz.y);
            drawList->AddRectFilled(min, max, IM_COL32(18, 20, 26, 255));
            drawList->AddRect(min, max, IM_COL32(50, 56, 68, 255));
            ImGui::Dummy(sz);
            ImGui::SetCursorPos(ImVec2(12, 28));
            ImGui::TextDisabled("Scene viewport unavailable");
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
#endif
}

// ---- Asset Browser ----
void EditorApp::drawAssetBrowser() {
#ifdef FELISS_HAS_IMGUI
    EditorPanels::DrawAssetBrowserPanel(m_engine, m_assetViewPath, m_iconSize);
#endif
}

// ---- Console ----
void EditorApp::drawConsole() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Console")) { ImGui::End(); return; }

    // Filter checkboxes
    const char* labels[] = {"TRC","DBG","INF","WRN","ERR","FTL"};
    for (int i = 0; i < 6; ++i) {
        ImGui::Checkbox(labels[i], &m_showLog[i]);
        if (i < 5) ImGui::SameLine();
    }
    ImGui::SameLine(0, 20);
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("Filter", m_consoleFilter, sizeof(m_consoleFilter));
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) Logger::get().clearEntries();
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_consoleAutoScroll);
    ImGui::Separator();

    ImGui::BeginChild("LogScroll", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (auto& e : Logger::get().entries()) {
        int lvl = (int)e.level;
        if (lvl < 0 || lvl > 5 || !m_showLog[lvl]) continue;
        if (m_consoleFilter[0] && e.message.find(m_consoleFilter) == std::string::npos) continue;
        ImVec4 col = ImVec4(1,1,1,1);
        switch(e.level) {
            case LogLevel::Trace:   col = ImVec4(.5f,.5f,.5f,1); break;
            case LogLevel::Debug:   col = ImVec4(.4f,.8f,.9f,1); break;
            case LogLevel::Info:    col = ImVec4(.8f,.9f,.8f,1); break;
            case LogLevel::Warning: col = ImVec4(1.f,.9f,.2f,1); break;
            case LogLevel::Error:   col = ImVec4(1.f,.4f,.4f,1); break;
            case LogLevel::Fatal:   col = ImVec4(.9f,.2f,.9f,1); break;
            default: break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(e.message.c_str());
        ImGui::PopStyleColor();
    }
    if (m_consoleAutoScroll) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::End();
#endif
}

// ---- Render Pipeline Editor ----
void EditorApp::drawRenderPipelineEditor() {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Render Pipeline")) { ImGui::End(); return; }

    RenderPipeline& rp = m_engine.renderer();
    ImGui::Text("API: %s", RenderAPIToString(rp.api()));
    ImGui::Text("Resolution: %dx%d", rp.width(), rp.height());
    ImGui::Separator();

    // Pipeline mode
    int mode = (int)rp.getPipelineMode();
    if (ImGui::Combo("Mode", &mode, "PBR 3D\0HD-2D / 2.5D\0Pixel Art 2D\0Toon\0Custom\0"))
        rp.setPipelineMode((PipelineMode)mode);

    ImGui::Separator();
    ImGui::Text("Render Passes:");
    for (auto& pass : rp.passes()) {
        bool en = pass.enabled;
        if (ImGui::Checkbox(pass.name.c_str(), &en))
            rp.setPassEnabled(pass.name, en);
    }

    ImGui::Separator();
    ImGui::Text("Post Effects:");
    for (auto& fx : rp.postEffects()) {
        bool en = fx.enabled;
        if (ImGui::Checkbox(fx.name.c_str(), &en))
            rp.setPostEffectEnabled(fx.name, en);
    }

    ImGui::Separator();
    const auto& st = rp.stats();
    ImGui::Text("Draw calls: %u", st.drawCalls);
    ImGui::Text("Sprites:    %u", st.spriteCalls);
    ImGui::Text("Triangles:  %u", st.triangles);
    ImGui::Text("Lights:     %u", st.lightCount);
    ImGui::Text("GPU ms:     %.2f", st.gpuMs);

    ImGui::End();
#endif
}

void EditorApp::createDefaultScene() {
    m_engine.world().clear();

    const EntityID cameraId = m_engine.world().createEntity("Main Camera");
    if (!m_engine.world().hasComponent<CameraComponent>(cameraId)) {
        auto& camera = m_engine.world().addComponent<CameraComponent>(cameraId);
        camera.isMain = true;
        camera.clearColor = {0.07f, 0.08f, 0.11f, 1.0f};
    }
    if (auto* transform = m_engine.world().getComponent<TransformComponent>(cameraId)) {
        transform->position = {0.0f, 0.0f, 2.5f};
        transform->dirty = true;
    }

    const EntityID lightId = m_engine.world().createEntity("Key Light");
    if (!m_engine.world().hasComponent<LightComponent>(lightId)) {
        auto& light = m_engine.world().addComponent<LightComponent>(lightId);
        light.type = LightComponent::LightType::Directional;
        light.intensity = 1.5f;
    }
    if (auto* transform = m_engine.world().getComponent<TransformComponent>(lightId)) {
        transform->position = {0.0f, 1.5f, 1.5f};
        transform->dirty = true;
    }

    const EntityID cubeId = m_engine.world().createEntity("Demo Triangle");
    {
        auto& mesh = m_engine.world().addComponent<MeshRendererComponent>(cubeId);
        mesh.meshID = 1;
        mesh.materialID = 1;
        mesh.castShadow = true;
        mesh.receiveShadow = true;
    }
    if (auto* transform = m_engine.world().getComponent<TransformComponent>(cubeId)) {
        transform->position = {0.0f, 0.0f, 0.0f};
        transform->scale = {0.65f, 0.65f, 0.65f};
        transform->dirty = true;
    }

    const EntityID spriteId = m_engine.world().createEntity("Demo Sprite");
    {
        auto& sprite = m_engine.world().addComponent<SpriteComponent>(spriteId);
        sprite.textureID = NULL_ASSET;
        sprite.tint = {0.95f, 0.55f, 0.25f, 0.90f};
        sprite.size = {0.35f, 0.35f};
        sprite.sortOrder = 1;
    }
    if (auto* transform = m_engine.world().getComponent<TransformComponent>(spriteId)) {
        transform->position = {-0.85f, -0.55f, 0.0f};
        transform->dirty = true;
    }

    m_selected = cameraId;
    FLS_INFO("Editor", "Created default scene");
}

void EditorApp::newScene() {
    createDefaultScene();
    m_scenePath = std::filesystem::path(m_engine.config().projectPath + "/scene.fls").lexically_normal().generic_string();
    FLS_INFOF("Editor", "New scene ready: " << m_scenePath);
}

bool EditorApp::openScene() {
    const auto scenePath = EditorFileDialogs::OpenSceneFile(m_scenePath);
    if (!scenePath) {
        return false;
    }

    m_scenePath = std::filesystem::path(*scenePath).lexically_normal().generic_string();
    const bool loaded = m_engine.world().loadFromFile(m_scenePath);
    if (!loaded) {
        FLS_ERRORF("Editor", "Failed to open scene: " << m_scenePath);
        return false;
    }

    m_selected = NULL_ENTITY;
    FLS_INFOF("Editor", "Opened scene: " << m_scenePath);
    return true;
}

bool EditorApp::saveScene(bool saveAs) {
    if (saveAs || m_scenePath.empty()) {
        const auto scenePath = EditorFileDialogs::SaveSceneFile(
            m_scenePath.empty() ? (m_engine.config().projectPath + "/scene.fls") : m_scenePath);
        if (!scenePath) {
            return false;
        }
        m_scenePath = std::filesystem::path(*scenePath).lexically_normal().generic_string();
    }

    const double start = Timer::GetTimeSeconds();
    const bool saved = m_engine.world().saveToFile(m_scenePath);
    FLS_INFOF("Editor", (saved ? "Saved" : "Failed to save") << " scene to " << m_scenePath
        << " in " << (Timer::GetTimeSeconds() - start) << "s");
    return saved;
}

void EditorApp::ensureEditorConfigFile() {
    if (FileSystem::Exists(m_editorConfigPath)) {
        return;
    }

    const bool created = FileSystem::WriteTextFile(m_editorConfigPath, buildEditorConfigText());
    FLS_INFOF("EditorConfig", (created ? "Created" : "Failed to create") << " default config: " << m_editorConfigPath);
}

void EditorApp::applyEditorConfig(const std::string& contents, bool logReload) {
    std::istringstream stream(contents);
    std::string line;
    bool sawAssetPath = false;

    while (std::getline(stream, line)) {
        const auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line.erase(commentPos);
        }

        const auto equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            continue;
        }

        auto key = line.substr(0, equalPos);
        auto value = line.substr(equalPos + 1);
        auto trim = [](std::string& text) {
            const auto begin = text.find_first_not_of(" \t\r\n");
            const auto end = text.find_last_not_of(" \t\r\n");
            if (begin == std::string::npos) {
                text.clear();
                return;
            }
            text = text.substr(begin, end - begin + 1);
        };
        trim(key);
        trim(value);

        if (key == "editor.show_grid") {
            m_showGrid = (value == "1" || value == "true" || value == "on");
        } else if (key == "editor.icon_size") {
            try {
                m_iconSize = std::stof(value);
            } catch (...) {
            }
        } else if (key == "editor.asset_path") {
            applyAssetPath(value, logReload);
            sawAssetPath = true;
        } else if (key == "log.min_level") {
            applyLoggerLevel(value, logReload);
        } else if (key == "editor.scene_path") {
            applyScenePath(value, logReload);
        }
    }

    if (!sawAssetPath || m_assetRootPath.empty()) {
        m_assetRootPath = std::filesystem::path(m_engine.config().projectPath + "/assets").lexically_normal().generic_string();
        m_assetViewPath = m_assetRootPath;
        m_engine.assets().setRootPath(m_assetRootPath);
        m_engine.assets().scan();
    }

    if (logReload) {
        FLS_INFOF("EditorConfig", "Applied hot-reloaded config from " << m_editorConfigPath
            << " | assetPath=" << m_assetRootPath
            << " | iconSize=" << m_iconSize
            << " | showGrid=" << (m_showGrid ? "true" : "false"));
    }
}

std::string EditorApp::buildEditorConfigText() const {
    std::ostringstream out;
    out << "# Feliss editor config\n";
    out << "editor.show_grid=" << (m_showGrid ? "true" : "false") << '\n';
    out << "editor.icon_size=" << m_iconSize << '\n';
    out << "editor.asset_path=" << m_assetRootPath << '\n';
    out << "editor.scene_path=" << m_scenePath << '\n';
    out << "log.min_level=" << m_logLevelName << '\n';
    return out.str();
}

void EditorApp::applyAssetPath(const std::string& value, bool logReload) {
    const std::string newPath = std::filesystem::path(value.empty()
        ? (m_engine.config().projectPath + "/assets")
        : value).lexically_normal().generic_string();

    std::error_code ec;
    const bool exists = std::filesystem::exists(newPath, ec);
    const bool isDirectory = exists && std::filesystem::is_directory(newPath, ec);
    m_assetRootPath = newPath;
    m_assetViewPath = m_assetRootPath;
    m_engine.assets().setRootPath(m_assetRootPath);
    m_engine.assets().scan();

    if (!logReload) {
        return;
    }

    if (!exists) {
        FLS_WARNF("EditorConfig", "Asset path hot-reloaded but does not exist: " << m_assetRootPath);
        return;
    }
    if (!isDirectory) {
        FLS_WARNF("EditorConfig", "Asset path hot-reloaded but is not a directory: " << m_assetRootPath);
        return;
    }

    FLS_INFOF("EditorConfig", "Asset browser root hot-reloaded to " << m_assetRootPath);
}

void EditorApp::triggerConfigReload() {
    std::string contents;
    if (!FileSystem::ReadTextFile(m_editorConfigPath, contents)) {
        FLS_WARNF("EditorConfig", "Failed to read config for reload: " << m_editorConfigPath);
        return;
    }

    applyEditorConfig(contents, true);
}

void EditorApp::applyLoggerLevel(const std::string& value, bool logReload) {
    std::string normalized = value;
    for (char& ch : normalized) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    LogLevel level = LogLevel::Debug;
    if (normalized == "trace") level = LogLevel::Trace;
    else if (normalized == "debug") level = LogLevel::Debug;
    else if (normalized == "info") level = LogLevel::Info;
    else if (normalized == "warning" || normalized == "warn") normalized = "warning", level = LogLevel::Warning;
    else if (normalized == "error") level = LogLevel::Error;
    else if (normalized == "fatal") level = LogLevel::Fatal;
    else normalized = "debug";

    Logger::get().setMinLevel(level);
    m_logLevelName = normalized;
    if (logReload) {
        FLS_INFOF("EditorConfig", "Logger level hot-reloaded to " << m_logLevelName);
    }
}

bool EditorApp::exportRuntimePackage() {
    const auto exportFolder = EditorFileDialogs::SelectFolder(m_engine.config().projectPath + "/export/windows");
    if (!exportFolder) {
        return false;
    }

    ProjectExportDesc desc;
    desc.projectPath = m_engine.config().projectPath;
    desc.buildPath = m_engine.config().projectPath + "/build-native";
    desc.outputPath = *exportFolder;
    desc.scenePath = m_scenePath;
    desc.assetRootPath = m_assetRootPath;

    std::string errorMessage;
    const bool exported = ProjectExporter::exportRuntimePackage(desc, &errorMessage);
    if (exported) {
        FLS_INFOF("Editor", "Exported runtime package to " << desc.outputPath);
    } else {
        FLS_ERRORF("Editor", "Export failed: " << errorMessage);
    }
    return exported;
}

void EditorApp::applyScenePath(const std::string& value, bool logReload) {
    if (value.empty()) {
        return;
    }

    m_scenePath = std::filesystem::path(value).lexically_normal().generic_string();
    if (!FileSystem::Exists(m_scenePath)) {
        if (logReload) {
            FLS_WARNF("EditorConfig", "Scene path updated but file does not exist: " << m_scenePath);
        }
        return;
    }

    const bool loaded = m_engine.world().loadFromFile(m_scenePath);
    if (logReload) {
        FLS_INFOF("EditorConfig", (loaded ? "Loaded" : "Failed to load")
            << " scene from hot-reloaded path: " << m_scenePath);
    }
}

} // namespace Feliss
