#pragma once

#include <functional>
#include <string>

namespace Feliss {

class ConfigHotReloadService {
public:
    using ReloadCallback = std::function<void(const std::string& path, const std::string& contents)>;

    bool start(const std::string& path, ReloadCallback callback);
    void stop();
    void poll();

    bool isWatching() const { return !m_path.empty(); }
    const std::string& path() const { return m_path; }

private:
    std::string   m_path;
    std::string   m_lastContents;
    double        m_lastPollTime = 0.0;
    long long     m_lastWriteStamp = -1;
    ReloadCallback m_callback;
};

} // namespace Feliss
