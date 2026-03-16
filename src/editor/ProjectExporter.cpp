#include "editor/ProjectExporter.h"

#include "core/FileSystem.h"

#include <filesystem>
#include <sstream>

namespace Feliss {

namespace {

std::string projectNameFromPath(const std::string& projectPath) {
    const std::filesystem::path path(projectPath);
    const auto name = path.filename().string();
    return name.empty() ? "FelissGame" : name;
}

void setError(std::string* errorMessage, const std::string& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

} // namespace

bool ProjectExporter::exportRuntimePackage(const ProjectExportDesc& desc, std::string* errorMessage) {
    const std::filesystem::path buildPath(desc.buildPath);
    const std::filesystem::path outputPath(desc.outputPath);
    const std::filesystem::path runtimeSource = buildPath / "feliss_runtime.exe";
    const std::filesystem::path engineCoreSource = buildPath / "EngineCore.dll";

    if (!FileSystem::Exists(runtimeSource.string())) {
        setError(errorMessage, "Missing runtime executable: " + runtimeSource.string());
        return false;
    }
    if (!FileSystem::EnsureDirectory(outputPath.string())) {
        setError(errorMessage, "Failed to create export directory: " + outputPath.string());
        return false;
    }
    if (!FileSystem::Exists(desc.scenePath)) {
        setError(errorMessage, "Scene file is missing: " + desc.scenePath);
        return false;
    }

    const std::string projectName = projectNameFromPath(desc.projectPath);
    const std::filesystem::path runtimeTarget = outputPath / (projectName + ".exe");
    if (!FileSystem::CopyFile(runtimeSource.string(), runtimeTarget.string())) {
        setError(errorMessage, "Failed to copy runtime executable");
        return false;
    }

    if (FileSystem::Exists(engineCoreSource.string()) &&
        !FileSystem::CopyFile(engineCoreSource.string(), (outputPath / "EngineCore.dll").string())) {
        setError(errorMessage, "Failed to copy EngineCore.dll");
        return false;
    }

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(buildPath, ec)) {
        if (ec || !entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".dll") {
            continue;
        }
        const auto fileName = entry.path().filename().string();
        if (fileName == "feliss.exe") {
            continue;
        }
        if (!FileSystem::CopyFile(entry.path().string(), (outputPath / fileName).string())) {
            setError(errorMessage, "Failed to copy dependency: " + fileName);
            return false;
        }
    }

    const std::filesystem::path extensionBuildRoot = buildPath / "ext";
    if (std::filesystem::is_directory(extensionBuildRoot, ec) &&
        !FileSystem::CopyDirectoryRecursive(extensionBuildRoot.string(), (outputPath / "extensions").string())) {
        setError(errorMessage, "Failed to copy built extensions");
        return false;
    }

    if (!FileSystem::CopyFile(desc.scenePath, (outputPath / "scene.fls").string())) {
        setError(errorMessage, "Failed to copy scene");
        return false;
    }

    if (FileSystem::Exists(desc.assetRootPath) &&
        !FileSystem::CopyDirectoryRecursive(desc.assetRootPath, (outputPath / "assets").string())) {
        setError(errorMessage, "Failed to copy assets");
        return false;
    }

    std::ostringstream manifest;
    manifest << "project=" << projectName << '\n';
    manifest << "runtime=" << runtimeTarget.filename().string() << '\n';
    manifest << "scene=scene.fls\n";
    manifest << "assets=assets\n";
    manifest << "extensions=extensions\n";
    if (!FileSystem::WriteTextFile((outputPath / "feliss_package.txt").string(), manifest.str())) {
        setError(errorMessage, "Failed to write package manifest");
        return false;
    }

    return true;
}

} // namespace Feliss
