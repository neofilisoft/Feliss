#pragma once

#include <string>

namespace Feliss {

struct ProjectExportDesc {
    std::string projectPath;
    std::string buildPath;
    std::string outputPath;
    std::string scenePath;
    std::string assetRootPath;
};

class ProjectExporter {
public:
    static bool exportRuntimePackage(const ProjectExportDesc& desc, std::string* errorMessage = nullptr);
};

} // namespace Feliss
