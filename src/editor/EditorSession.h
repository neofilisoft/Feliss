#pragma once

#include "feliss/Types.h"

namespace Feliss {

class Engine;
class EditorApp;

class EditorSession {
public:
    explicit EditorSession(Engine& engine);
    ~EditorSession();
    EditorSession(const EditorSession&) = delete;
    EditorSession& operator=(const EditorSession&) = delete;

    bool init();
    void shutdown();

private:
    void renderFrame();

    Engine& m_engine;
    Scope<EditorApp> m_editor;
    bool m_initialized = false;
};

} // namespace Feliss
