#pragma once

#include "core/Engine.h"

#include <string>

namespace Feliss {

class RuntimeApplication {
public:
    explicit RuntimeApplication(EngineConfig config);
    ~RuntimeApplication();
    RuntimeApplication(const RuntimeApplication&) = delete;
    RuntimeApplication& operator=(const RuntimeApplication&) = delete;

    bool init(const std::string& bootScenePath = {});
    int run();
    void shutdown();

    Engine& engine();

private:
    EngineConfig m_config;
    std::string m_bootScenePath;
    Scope<Engine> m_engine;
};

} // namespace Feliss
