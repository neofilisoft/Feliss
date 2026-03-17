#include "editor/PythonTooling.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>

namespace Feliss {

PythonTooling::PythonTooling(std::string toolsRoot)
    : m_toolsRoot(std::move(toolsRoot)) {}

void PythonTooling::setToolsRoot(std::string toolsRoot) {
    m_toolsRoot = std::move(toolsRoot);
}

std::vector<PythonToolScript> PythonTooling::discoverScripts() const {
    std::vector<PythonToolScript> scripts;
    std::error_code ec;
    if (!std::filesystem::is_directory(m_toolsRoot, ec)) {
        return scripts;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_toolsRoot, ec)) {
        if (ec || !entry.is_regular_file() || entry.path().extension() != ".py") {
            continue;
        }
        scripts.push_back({entry.path().stem().string(), entry.path().generic_string()});
    }

    std::sort(scripts.begin(), scripts.end(), [](const PythonToolScript& lhs, const PythonToolScript& rhs) {
        return lhs.name < rhs.name;
    });
    return scripts;
}

std::string PythonTooling::findInterpreter() const {
    static std::string cachedInterpreter;
    static bool resolved = false;
    if (resolved) {
        return cachedInterpreter;
    }

#ifdef _WIN32
    if (std::system("py -3 -V >nul 2>nul") == 0) {
        cachedInterpreter = "py -3";
        resolved = true;
        return cachedInterpreter;
    }
    if (std::system("python --version >nul 2>nul") == 0) {
        cachedInterpreter = "python";
        resolved = true;
        return cachedInterpreter;
    }
#else
    if (std::system("python3 --version >/dev/null 2>&1") == 0) {
        cachedInterpreter = "python3";
        resolved = true;
        return cachedInterpreter;
    }
    if (std::system("python --version >/dev/null 2>&1") == 0) {
        cachedInterpreter = "python";
        resolved = true;
        return cachedInterpreter;
    }
#endif
    resolved = true;
    return cachedInterpreter;
}

bool PythonTooling::runScript(const std::string& scriptPath, const std::vector<std::string>& args, std::string* errorMessage) const {
    const std::string interpreter = findInterpreter();
    if (interpreter.empty()) {
        if (errorMessage) {
            *errorMessage = "Python interpreter not found";
        }
        return false;
    }

    std::string command = interpreter + " \"" + scriptPath + "\"";
    for (const auto& arg : args) {
        command += " \"" + arg + "\"";
    }
    const int code = std::system(command.c_str());
    if (code != 0 && errorMessage) {
        *errorMessage = "Python tool failed with exit code " + std::to_string(code);
    }
    return code == 0;
}

} // namespace Feliss
