#pragma once

#include <string>

namespace Feliss::FileSystem {

bool Exists(const std::string& path);
bool ReadTextFile(const std::string& path, std::string& outText);
bool WriteTextFile(const std::string& path, const std::string& text);
bool EnsureDirectory(const std::string& path);
bool CopyFile(const std::string& sourcePath, const std::string& destinationPath);
bool CopyDirectoryRecursive(const std::string& sourcePath, const std::string& destinationPath);

} // namespace Feliss::FileSystem
