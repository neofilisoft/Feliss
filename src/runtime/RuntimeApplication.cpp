#include "runtime/RuntimeApplication.h"

#include "core/Engine.h"
#include "core/FileSystem.h"
#include "core/Logger.h"

namespace Feliss {

RuntimeApplication::RuntimeApplication(EngineConfig config)
    : m_config(std::move(config)) {
    m_config.mode = EngineMode::Game;
}

RuntimeApplication::~RuntimeApplication() {
    shutdown();
}

bool RuntimeApplication::init(const std::string& bootScenePath) {
    if (m_engine) {
        return true;
    }

    m_bootScenePath = bootScenePath;
    m_engine = MakeScope<Engine>(m_config);
    if (!m_engine->init()) {
        return false;
    }

    if (!m_bootScenePath.empty()) {
        if (FileSystem::Exists(m_bootScenePath)) {
            if (!m_engine->world().loadFromFile(m_bootScenePath)) {
                FLS_WARNF("Runtime", "Failed to load boot scene: " << m_bootScenePath);
            }
        } else {
            FLS_WARNF("Runtime", "Boot scene does not exist: " << m_bootScenePath);
        }
    }

    return true;
}

int RuntimeApplication::run() {
    if (!m_engine) {
        return 1;
    }

    m_engine->run();
    return 0;
}

void RuntimeApplication::shutdown() {
    if (!m_engine) {
        return;
    }

    m_engine->shutdown();
    m_engine.reset();
}

Engine& RuntimeApplication::engine() {
    return *m_engine;
}

} // namespace Feliss
