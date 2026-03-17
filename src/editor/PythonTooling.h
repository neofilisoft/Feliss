#pragma once

#include <string>
#include <vector>

namespace Feliss {

struct PythonToolScript {
    std::string name;
    std::string path;
};

class PythonTooling {
public:
    explicit PythonTooling(std::string toolsRoot);

    void setToolsRoot(std::string toolsRoot);
    const std::string& toolsRoot() const { return m_toolsRoot; }

    std::vector<PythonToolScript> discoverScripts() const;
    std::string findInterpreter() const;
    bool runScript(const std::string& scriptPath, const std::vector<std::string>& args, std::string* errorMessage = nullptr) const;

private:
    std::string m_toolsRoot;
};

} // namespace Feliss
