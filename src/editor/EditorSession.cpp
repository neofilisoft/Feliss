#include "editor/EditorSession.h"

#include "core/Engine.h"
#include "core/Logger.h"
#include "editor/EditorApp.h"

#ifdef FELISS_HAS_IMGUI
#  include <imgui.h>
#  include <backends/imgui_impl_glfw.h>
#  include <backends/imgui_impl_opengl3.h>
#endif

#ifdef FELISS_HAS_GLFW
#  include <GLFW/glfw3.h>
#endif

namespace Feliss {

EditorSession::EditorSession(Engine& engine)
    : m_engine(engine) {}

EditorSession::~EditorSession() {
    shutdown();
}

bool EditorSession::init() {
#if defined(FELISS_HAS_IMGUI) && defined(FELISS_HAS_GLFW)
    if (m_initialized) {
        return true;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto* window = static_cast<GLFWwindow*>(m_engine.window().nativeHandle());
    if (!window) {
        FLS_ERROR("EditorSession", "GLFW window handle is null");
        return false;
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        FLS_ERROR("EditorSession", "ImGui GLFW init failed");
        ImGui::DestroyContext();
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        FLS_ERROR("EditorSession", "ImGui OpenGL3 init failed");
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    m_editor = MakeScope<EditorApp>(m_engine);
    if (!m_editor->init()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    m_engine.setRenderCallback([this]() { renderFrame(); });
    m_initialized = true;
    FLS_INFO("EditorSession", "Editor session initialised");
    return true;
#else
    FLS_ERROR("EditorSession", "Editor session requires ImGui + GLFW");
    return false;
#endif
}

void EditorSession::shutdown() {
#if defined(FELISS_HAS_IMGUI) && defined(FELISS_HAS_GLFW)
    if (!m_initialized) {
        return;
    }

    m_engine.setRenderCallback({});

    if (m_editor) {
        m_editor->shutdown();
        m_editor.reset();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
    FLS_INFO("EditorSession", "Editor session shutdown");
#endif
}

void EditorSession::renderFrame() {
#if defined(FELISS_HAS_IMGUI) && defined(FELISS_HAS_GLFW)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_editor) {
        m_editor->render();
    }
    m_engine.extensions().onImGui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

} // namespace Feliss
