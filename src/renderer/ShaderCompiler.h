#pragma once

#include "feliss/Types.h"

#include <string>
#include <vector>

namespace Feliss {

enum class ShaderBinaryFormat : u32 {
    Unknown = 0,
    SPIRV,
    DXIL,
    MetalLib,
};

struct ShaderCompileRequest {
    std::string sourcePath;
    std::string entryPoint = "main";
    std::string profile;
    ShaderBinaryFormat outputFormat = ShaderBinaryFormat::Unknown;
    std::vector<std::string> defines;
    bool forceRebuild = false;
};

struct ShaderCompileResult {
    bool ok = false;
    std::string outputPath;
    std::string compilerPath;
    std::string message;
};

class ShaderCompiler {
public:
    explicit ShaderCompiler(std::string cacheDirectory = "cache/shaders");

    const std::string& cacheDirectory() const { return m_cacheDirectory; }
    std::string findDXC() const;
    std::string findGLSLC() const;
    ShaderCompileResult compile(const ShaderCompileRequest& request) const;

private:
    std::string findTool(const char* toolName) const;
    std::string hashKey(const ShaderCompileRequest& request, const std::string& sourceText) const;
    std::string buildOutputPath(const ShaderCompileRequest& request, const std::string& hash) const;
    ShaderCompileResult writePlaceholderBinary(const ShaderCompileRequest& request, const std::string& compilerPath) const;

    std::string m_cacheDirectory;
};

} // namespace Feliss
