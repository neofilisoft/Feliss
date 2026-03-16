#pragma once

#include <optional>
#include <string>

namespace Feliss::EditorFileDialogs {

std::optional<std::string> OpenSceneFile(const std::string& initialPath);
std::optional<std::string> SaveSceneFile(const std::string& initialPath);
std::optional<std::string> SelectFolder(const std::string& initialPath);

} // namespace Feliss::EditorFileDialogs
