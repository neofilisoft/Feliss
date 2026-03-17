#include "project/ProjectConfig.h"

#include "core/FileSystem.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace Feliss {

namespace {

std::string trim(std::string value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::string normalize(std::string value) {
    value = trim(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool parseBool(const std::string& value) {
    const std::string normalized = normalize(value);
    return normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "on";
}

const char* renderApiToConfig(RenderAPI api) {
    switch (api) {
        case RenderAPI::Vulkan:    return "vulkan";
        case RenderAPI::DirectX11: return "dx11";
        case RenderAPI::DirectX12: return "dx12";
        case RenderAPI::Metal:     return "metal";
        default:                   return "opengl";
    }
}

RenderAPI renderApiFromConfig(const std::string& value) {
    const std::string normalized = normalize(value);
    if (normalized == "vulkan") return RenderAPI::Vulkan;
    if (normalized == "dx11")   return RenderAPI::DirectX11;
    if (normalized == "dx12")   return RenderAPI::DirectX12;
    if (normalized == "metal")  return RenderAPI::Metal;
    return RenderAPI::OpenGL;
}

} // namespace

bool LoadProjectConfig(const std::string& path, ProjectConfig& outConfig) {
    std::string contents;
    if (!FileSystem::ReadTextFile(path, contents)) {
        return false;
    }

    std::istringstream stream(contents);
    std::string line;
    while (std::getline(stream, line)) {
        const auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line.erase(commentPos);
        }

        const auto separator = line.find(':');
        if (separator == std::string::npos) {
            continue;
        }

        const std::string key = normalize(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));

        if (key == "script.default_language") outConfig.defaultScriptLanguage = ScriptLanguageFromString(normalize(value));
        else if (key == "script.enable_csharp") outConfig.enableCSharp = parseBool(value);
        else if (key == "script.enable_lua") outConfig.enableLua = parseBool(value);
        else if (key == "render.preferred_api") outConfig.preferredRenderAPI = renderApiFromConfig(value);
        else if (key == "physics.backend") outConfig.physicsBackend = PhysicsBackendTypeFromString(normalize(value));
        else if (key == "project.default_scene") outConfig.defaultScene = value;
        else if (key == "project.asset_root") outConfig.assetRoot = value;
    }

    return true;
}

bool SaveProjectConfig(const std::string& path, const ProjectConfig& config) {
    std::ostringstream out;
    out << "script.default_language: " << (config.defaultScriptLanguage == ScriptLanguage::Lua ? "lua" : "csharp") << '\n';
    out << "script.enable_csharp: " << (config.enableCSharp ? "true" : "false") << '\n';
    out << "script.enable_lua: " << (config.enableLua ? "true" : "false") << '\n';
    out << "render.preferred_api: " << renderApiToConfig(config.preferredRenderAPI) << '\n';
    out << "physics.backend: " << normalize(PhysicsBackendTypeToString(config.physicsBackend)) << '\n';
    out << "project.default_scene: " << config.defaultScene << '\n';
    out << "project.asset_root: " << config.assetRoot << '\n';
    return FileSystem::WriteTextFile(path, out.str());
}

} // namespace Feliss
