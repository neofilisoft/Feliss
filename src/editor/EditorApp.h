#pragma once
#include "feliss/Types.h"
#include "editor/ConfigHotReloadService.h"
#include <string>
#include <functional>

namespace Feliss {

class Engine;

// =====================================================================
// EditorApp — ImGui-based visual editor
// Panels: Hierarchy, Inspector, SceneView, AssetBrowser, Console,
//         RenderPipeline, Extensions
// =====================================================================
enum class PlayState { Stopped, Playing, Paused };
enum class GizmoTool { Select=0, Move, Rotate, Scale };

class EditorApp {
public:
    explicit EditorApp(Engine& engine);
    ~EditorApp() = default;

    bool init();
    void shutdown();
    void render();          // call inside ImGui frame

    PlayState  playState()   const { return m_playState; }
    GizmoTool  activeTool()  const { return m_tool; }
    EntityID   selected()    const { return m_selected; }

private:
    void drawMainMenuBar();
    void drawToolbar();
    void drawDockspace();
    void drawHierarchy();
    void drawInspector();
    void drawSceneView();
    void drawAssetBrowser();
    void drawConsole();
    void drawRenderPipelineEditor();
    void ensureEditorConfigFile();
    void createDefaultScene();
    void newScene();
    bool openScene();
    bool saveScene(bool saveAs);
    bool exportRuntimePackage();
    void applyEditorConfig(const std::string& contents, bool logReload);
    std::string buildEditorConfigText() const;
    void triggerConfigReload();
    void applyLoggerLevel(const std::string& value, bool logReload);
    void applyAssetPath(const std::string& value, bool logReload);
    void applyScenePath(const std::string& value, bool logReload);

    void drawEntityNode(EntityID id);

    Engine&   m_engine;
    PlayState m_playState = PlayState::Stopped;
    GizmoTool m_tool      = GizmoTool::Move;
    EntityID  m_selected  = NULL_ENTITY;

    // UI state
    char       m_renameBuffer[256] = {};
    bool       m_renaming          = false;
    EntityID   m_renamingID        = NULL_ENTITY;
    std::string m_assetRootPath    = ".";
    std::string m_assetViewPath    = ".";
    char       m_consoleFilter[128]= {};
    bool       m_consoleAutoScroll = true;
    bool       m_showLog[6]        = {true,true,true,true,true,true};

    float m_iconSize  = 64.0f;
    bool  m_showGrid  = true;
    std::string m_editorConfigPath;
    std::string m_scenePath;
    std::string m_logLevelName = "debug";
    ConfigHotReloadService m_configHotReload;
};

} // namespace Feliss
