#pragma once

#include "feliss/Types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Feliss {

struct AssetRecord {
    UUID guid;
    std::string sourcePath;
    std::string metaPath;
    std::string importer = "raw";
    std::vector<std::string> dependencies;
};

class AssetRegistry {
public:
    explicit AssetRegistry(std::string rootPath);

    void setRootPath(std::string rootPath);
    bool scan();
    const AssetRecord* findByPath(const std::string& sourcePath) const;
    const AssetRecord* findByGuid(const UUID& guid) const;
    std::vector<const AssetRecord*> all() const;
    const std::string& rootPath() const { return m_rootPath; }

private:
    bool ensureMeta(const std::string& sourcePath, AssetRecord& record);
    bool loadMeta(const std::string& metaPath, AssetRecord& record) const;
    bool writeMeta(const AssetRecord& record) const;

    std::string m_rootPath;
    std::unordered_map<std::string, AssetRecord> m_byPath;
    std::unordered_map<UUID, std::string> m_guidToPath;
};

} // namespace Feliss
