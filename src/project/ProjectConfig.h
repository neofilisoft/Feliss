#pragma once

#include "feliss/Types.h"
#include "physics/PhysicsTypes.h"

#include <string>

namespace Feliss {

enum class ScriptLanguage : u32 {
    CSharp = 0,
    Lua,
};

inline const char* ScriptLanguageToString(ScriptLanguage language) {
    return language == ScriptLanguage::Lua ? "Lua" : "CSharp";
}

inline ScriptLanguage ScriptLanguageFromString(const std::string& value) {
    return value == "lua" ? ScriptLanguage::Lua : ScriptLanguage::CSharp;
}

struct ProjectConfig {
    ScriptLanguage defaultScriptLanguage = ScriptLanguage::CSharp;
    bool enableCSharp = true;
    bool enableLua = false;
    RenderAPI preferredRenderAPI = RenderAPI::OpenGL;
    PhysicsBackendType physicsBackend = PhysicsBackendType::AsterCore;
    std::string defaultScene = "./scene.fls";
    std::string assetRoot = "./assets";
};

bool LoadProjectConfig(const std::string& path, ProjectConfig& outConfig);
bool SaveProjectConfig(const std::string& path, const ProjectConfig& config);

} // namespace Feliss
