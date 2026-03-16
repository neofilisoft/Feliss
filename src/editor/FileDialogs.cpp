#include "editor/FileDialogs.h"

#include "core/Logger.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <commdlg.h>
#  include <shlobj.h>
#endif

#include <array>
#include <cstring>
#include <filesystem>

namespace Feliss::EditorFileDialogs {

#ifdef _WIN32
namespace {

std::optional<std::string> runFileDialog(bool saveDialog, const std::string& initialPath) {
    std::array<char, MAX_PATH> buffer {};
    if (!initialPath.empty()) {
        const size_t count = (std::min)(initialPath.size(), buffer.size() - 1);
        memcpy(buffer.data(), initialPath.data(), count);
        buffer[count] = '\0';
    }

    OPENFILENAMEA dialog {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = nullptr;
    dialog.lpstrFile = buffer.data();
    dialog.nMaxFile = static_cast<DWORD>(buffer.size());
    dialog.lpstrFilter = "Feliss Scene (*.fls)\0*.fls\0All Files (*.*)\0*.*\0";
    dialog.nFilterIndex = 1;
    dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    dialog.lpstrDefExt = "fls";

    std::string initialDirectory;
    if (!initialPath.empty()) {
        std::error_code ec;
        std::filesystem::path path(initialPath);
        const std::filesystem::path folder = path.has_parent_path() ? path.parent_path() : path;
        if (!folder.empty() && std::filesystem::exists(folder, ec)) {
            initialDirectory = folder.string();
            dialog.lpstrInitialDir = initialDirectory.c_str();
        }
    }

    const BOOL result = saveDialog ? GetSaveFileNameA(&dialog) : GetOpenFileNameA(&dialog);
    if (!result) {
        return std::nullopt;
    }
    return std::string(buffer.data());
}

} // namespace
#endif

std::optional<std::string> OpenSceneFile(const std::string& initialPath) {
#ifdef _WIN32
    return runFileDialog(false, initialPath);
#else
    FLS_WARN("Editor", "OpenSceneFile is only implemented on Windows right now");
    (void)initialPath;
    return std::nullopt;
#endif
}

std::optional<std::string> SaveSceneFile(const std::string& initialPath) {
#ifdef _WIN32
    return runFileDialog(true, initialPath);
#else
    FLS_WARN("Editor", "SaveSceneFile is only implemented on Windows right now");
    (void)initialPath;
    return std::nullopt;
#endif
}

std::optional<std::string> SelectFolder(const std::string& initialPath) {
#ifdef _WIN32
    (void)initialPath;
    BROWSEINFOA browseInfo {};
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    browseInfo.lpszTitle = "Select export folder";

    PIDLIST_ABSOLUTE itemIdList = SHBrowseForFolderA(&browseInfo);
    if (!itemIdList) {
        return std::nullopt;
    }

    std::array<char, MAX_PATH> buffer {};
    const bool success = SHGetPathFromIDListA(itemIdList, buffer.data()) == TRUE;
    CoTaskMemFree(itemIdList);
    if (!success) {
        return std::nullopt;
    }
    return std::string(buffer.data());
#else
    FLS_WARN("Editor", "SelectFolder is only implemented on Windows right now");
    (void)initialPath;
    return std::nullopt;
#endif
}

} // namespace Feliss::EditorFileDialogs
