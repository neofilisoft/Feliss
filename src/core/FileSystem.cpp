#include "core/FileSystem.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Feliss::FileSystem {

bool Exists(const std::string& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec) && !ec;
}

bool ReadTextFile(const std::string& path, std::string& outText) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    outText = buffer.str();
    return input.good() || input.eof();
}

bool WriteTextFile(const std::string& path, const std::string& text) {
    const std::filesystem::path fsPath(path);
    const auto parent = fsPath.parent_path();
    if (!parent.empty() && !EnsureDirectory(parent.string())) {
        return false;
    }

    std::ofstream output(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    return static_cast<bool>(output);
}

bool EnsureDirectory(const std::string& path) {
    std::error_code ec;
    if (path.empty()) {
        return false;
    }

    const std::filesystem::path fsPath(path);
    if (std::filesystem::exists(fsPath, ec)) {
        return std::filesystem::is_directory(fsPath, ec) && !ec;
    }

    std::filesystem::create_directories(fsPath, ec);
    return !ec;
}

bool CopyFile(const std::string& sourcePath, const std::string& destinationPath) {
    std::error_code ec;
    const std::filesystem::path destination(destinationPath);
    const auto parent = destination.parent_path();
    if (!parent.empty() && !EnsureDirectory(parent.string())) {
        return false;
    }

    std::filesystem::copy_file(sourcePath, destinationPath,
        std::filesystem::copy_options::overwrite_existing, ec);
    return !ec;
}

bool CopyDirectoryRecursive(const std::string& sourcePath, const std::string& destinationPath) {
    std::error_code ec;
    if (!std::filesystem::is_directory(sourcePath, ec)) {
        return false;
    }
    if (!EnsureDirectory(destinationPath)) {
        return false;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(sourcePath, ec)) {
        if (ec) {
            return false;
        }

        const auto relative = std::filesystem::relative(entry.path(), sourcePath, ec);
        if (ec) {
            return false;
        }

        const auto target = std::filesystem::path(destinationPath) / relative;
        if (entry.is_directory()) {
            if (!EnsureDirectory(target.string())) {
                return false;
            }
            continue;
        }
        if (entry.is_regular_file()) {
            if (!CopyFile(entry.path().string(), target.string())) {
                return false;
            }
        }
    }

    return true;
}

} // namespace Feliss::FileSystem
