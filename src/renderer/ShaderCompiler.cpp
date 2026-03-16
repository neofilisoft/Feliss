#include "renderer/ShaderCompiler.h"

#include "core/FileSystem.h"
#include "core/Logger.h"

#include <cstdlib>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace Feliss {

namespace {

const char* binaryExtension(ShaderBinaryFormat format) {
    switch (format) {
        case ShaderBinaryFormat::SPIRV: return ".spv";
        case ShaderBinaryFormat::DXIL: return ".dxil";
        case ShaderBinaryFormat::MetalLib: return ".metallib";
        default: return ".bin";
    }
}

const char* compilerName(ShaderBinaryFormat format) {
    switch (format) {
        case ShaderBinaryFormat::SPIRV: return "glslc";
        case ShaderBinaryFormat::DXIL: return "dxc";
        case ShaderBinaryFormat::MetalLib: return "metal";
        default: return "unknown";
    }
}

} // namespace

ShaderCompiler::ShaderCompiler(std::string cacheDirectory)
    : m_cacheDirectory(std::move(cacheDirectory)) {}

std::string ShaderCompiler::findDXC() const {
    return findTool("dxc.exe");
}

std::string ShaderCompiler::findGLSLC() const {
    return findTool("glslc.exe");
}

ShaderCompileResult ShaderCompiler::compile(const ShaderCompileRequest& request) const {
    std::string sourceText;
    if (!FileSystem::ReadTextFile(request.sourcePath, sourceText)) {
        return {false, {}, {}, "Failed to read shader source: " + request.sourcePath};
    }

    std::string compilerPath;
    switch (request.outputFormat) {
        case ShaderBinaryFormat::SPIRV:
            compilerPath = findGLSLC();
            break;
        case ShaderBinaryFormat::DXIL:
            compilerPath = findDXC();
            break;
        case ShaderBinaryFormat::MetalLib:
            compilerPath = findTool("metal");
            break;
        default:
            return {false, {}, {}, "Unsupported shader binary format"};
    }

    if (compilerPath.empty()) {
        return {false, {}, {}, std::string("Missing shader compiler in PATH: ") + compilerName(request.outputFormat)};
    }

    return writePlaceholderBinary(request, compilerPath);
}

std::string ShaderCompiler::findTool(const char* toolName) const {
#ifdef _WIN32
    char buffer[MAX_PATH] = {};
    const DWORD result = SearchPathA(nullptr, toolName, nullptr, MAX_PATH, buffer, nullptr);
    if (result > 0 && result < MAX_PATH) {
        return buffer;
    }
#else
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv) {
        return {};
    }

    std::stringstream stream(pathEnv);
    std::string pathEntry;
    while (std::getline(stream, pathEntry, ':')) {
        const std::filesystem::path candidate = std::filesystem::path(pathEntry) / toolName;
        std::error_code ec;
        if (std::filesystem::exists(candidate, ec) && !ec) {
            return candidate.string();
        }
    }
#endif
    return {};
}

std::string ShaderCompiler::hashKey(const ShaderCompileRequest& request, const std::string& sourceText) const {
    std::ostringstream hashSeed;
    hashSeed << request.sourcePath << '|'
             << request.entryPoint << '|'
             << request.profile << '|'
             << static_cast<u32>(request.outputFormat) << '|'
             << sourceText;
    for (const auto& define : request.defines) {
        hashSeed << '|' << define;
    }

    const auto hashValue = std::hash<std::string>{}(hashSeed.str());
    std::ostringstream out;
    out << std::hex << hashValue;
    return out.str();
}

std::string ShaderCompiler::buildOutputPath(const ShaderCompileRequest& request, const std::string& hash) const {
    const std::filesystem::path sourcePath(request.sourcePath);
    const std::string fileStem = sourcePath.stem().string();
    return (std::filesystem::path(m_cacheDirectory) / (fileStem + "_" + hash + binaryExtension(request.outputFormat))).string();
}

ShaderCompileResult ShaderCompiler::writePlaceholderBinary(const ShaderCompileRequest& request, const std::string& compilerPath) const {
    std::string sourceText;
    FileSystem::ReadTextFile(request.sourcePath, sourceText);

    const std::string hash = hashKey(request, sourceText);
    const std::string outputPath = buildOutputPath(request, hash);
    if (!request.forceRebuild && FileSystem::Exists(outputPath)) {
        return {true, outputPath, compilerPath, "Shader binary cache hit"};
    }

    std::ostringstream payload;
    payload << "feliss_shader_cache_v1\n";
    payload << "source=" << request.sourcePath << '\n';
    payload << "entry=" << request.entryPoint << '\n';
    payload << "profile=" << request.profile << '\n';
    payload << "format=" << static_cast<u32>(request.outputFormat) << '\n';
    payload << "compiler=" << compilerPath << '\n';
    for (const auto& define : request.defines) {
        payload << "define=" << define << '\n';
    }

    if (!FileSystem::WriteTextFile(outputPath, payload.str())) {
        return {false, {}, compilerPath, "Failed to write shader cache payload: " + outputPath};
    }

    FLS_INFOF("ShaderCompiler", "Prepared shader cache artifact: " << outputPath);
    return {true, outputPath, compilerPath, "Shader cache artifact prepared"};
}

} // namespace Feliss
