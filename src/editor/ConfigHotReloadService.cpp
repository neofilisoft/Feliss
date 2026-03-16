#include "editor/ConfigHotReloadService.h"

#include "core/FileSystem.h"
#include "core/Logger.h"
#include "core/Timer.h"

#include <filesystem>

namespace Feliss {

namespace {

long long ReadWriteStamp(const std::string& path) {
    std::error_code ec;
    const auto writeTime = std::filesystem::last_write_time(path, ec);
    if (ec) {
        return -1;
    }

    return writeTime.time_since_epoch().count();
}

} // namespace

bool ConfigHotReloadService::start(const std::string& path, ReloadCallback callback) {
    m_path = path;
    m_callback = std::move(callback);
    m_lastPollTime = 0.0;
    m_lastWriteStamp = -1;
    m_lastContents.clear();

    if (!FileSystem::Exists(m_path)) {
        FLS_WARNF("EditorConfig", "Config file is not present yet: " << m_path);
        return false;
    }

    m_lastWriteStamp = ReadWriteStamp(m_path);
    FileSystem::ReadTextFile(m_path, m_lastContents);
    FLS_INFOF("EditorConfig", "Watching config file: " << m_path);
    return true;
}

void ConfigHotReloadService::stop() {
    m_path.clear();
    m_lastContents.clear();
    m_lastPollTime = 0.0;
    m_lastWriteStamp = -1;
    m_callback = {};
}

void ConfigHotReloadService::poll() {
    if (m_path.empty()) {
        return;
    }

    const double now = Timer::GetTimeSeconds();
    if ((now - m_lastPollTime) < 0.25) {
        return;
    }
    m_lastPollTime = now;

    if (!FileSystem::Exists(m_path)) {
        return;
    }

    const long long writeStamp = ReadWriteStamp(m_path);
    if (writeStamp == m_lastWriteStamp) {
        return;
    }

    std::string contents;
    if (!FileSystem::ReadTextFile(m_path, contents)) {
        FLS_WARNF("EditorConfig", "Failed to read config file during hot reload: " << m_path);
        return;
    }

    m_lastWriteStamp = writeStamp;
    if (contents == m_lastContents) {
        return;
    }

    m_lastContents = contents;
    FLS_INFOF("EditorConfig", "Detected config change: " << m_path);
    if (m_callback) {
        m_callback(m_path, m_lastContents);
    }
}

} // namespace Feliss
