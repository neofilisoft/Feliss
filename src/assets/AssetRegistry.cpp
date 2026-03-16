#include "assets/AssetRegistry.h"

#include "core/FileSystem.h"
#include "core/Logger.h"

#include <filesystem>
#include <sstream>

namespace Feliss {

AssetRegistry::AssetRegistry(std::string rootPath)
    : m_rootPath(std::move(rootPath)) {}

void AssetRegistry::setRootPath(std::string rootPath) {
    m_rootPath = std::move(rootPath);
}

bool AssetRegistry::scan() {
    m_byPath.clear();
    m_guidToPath.clear();

    std::error_code ec;
    if (!std::filesystem::is_directory(m_rootPath, ec)) {
        FLS_WARNF("AssetRegistry", "Asset root missing: " << m_rootPath);
        return false;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_rootPath, ec)) {
        if (ec || !entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() == ".meta") {
            continue;
        }

        AssetRecord record;
        record.sourcePath = entry.path().generic_string();
        record.metaPath = record.sourcePath + ".meta";
        if (!ensureMeta(record.sourcePath, record)) {
            continue;
        }

        m_guidToPath[record.guid] = record.sourcePath;
        m_byPath[record.sourcePath] = std::move(record);
    }

    FLS_INFOF("AssetRegistry", "Scanned " << m_byPath.size() << " assets from " << m_rootPath);
    return true;
}

const AssetRecord* AssetRegistry::findByPath(const std::string& sourcePath) const {
    const auto it = m_byPath.find(sourcePath);
    return it != m_byPath.end() ? &it->second : nullptr;
}

const AssetRecord* AssetRegistry::findByGuid(const UUID& guid) const {
    const auto it = m_guidToPath.find(guid);
    if (it == m_guidToPath.end()) {
        return nullptr;
    }
    return findByPath(it->second);
}

std::vector<const AssetRecord*> AssetRegistry::all() const {
    std::vector<const AssetRecord*> records;
    records.reserve(m_byPath.size());
    for (const auto& [_, record] : m_byPath) {
        records.push_back(&record);
    }
    return records;
}

bool AssetRegistry::ensureMeta(const std::string& sourcePath, AssetRecord& record) {
    if (!FileSystem::Exists(record.metaPath)) {
        record.guid = UUID::generate();
        record.importer = "raw";
        return writeMeta(record);
    }

    if (!loadMeta(record.metaPath, record)) {
        FLS_WARNF("AssetRegistry", "Failed to load meta: " << record.metaPath);
        return false;
    }
    record.sourcePath = sourcePath;
    return true;
}

bool AssetRegistry::loadMeta(const std::string& metaPath, AssetRecord& record) const {
    std::string contents;
    if (!FileSystem::ReadTextFile(metaPath, contents)) {
        return false;
    }

    std::istringstream stream(contents);
    std::string line;
    while (std::getline(stream, line)) {
        const auto sep = line.find('=');
        if (sep == std::string::npos) {
            continue;
        }

        const std::string key = line.substr(0, sep);
        const std::string value = line.substr(sep + 1);
        if (key == "guid") {
            if (value.size() == 32) {
                record.guid.hi = std::stoull(value.substr(0, 16), nullptr, 16);
                record.guid.lo = std::stoull(value.substr(16, 16), nullptr, 16);
            }
        } else if (key == "importer") {
            record.importer = value;
        } else if (key == "dependency" && !value.empty()) {
            record.dependencies.push_back(value);
        }
    }

    return !record.guid.isNull();
}

bool AssetRegistry::writeMeta(const AssetRecord& record) const {
    std::ostringstream out;
    out << "guid=" << record.guid.toString() << '\n';
    out << "importer=" << record.importer << '\n';
    for (const auto& dependency : record.dependencies) {
        out << "dependency=" << dependency << '\n';
    }

    return FileSystem::WriteTextFile(record.metaPath, out.str());
}

} // namespace Feliss
