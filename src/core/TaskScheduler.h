#pragma once

#include "feliss/Types.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Feliss {

class TaskScheduler {
public:
    using Task = std::function<void()>;

    TaskScheduler() = default;
    ~TaskScheduler();
    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    bool start(u32 workerCount = 0);
    void stop();
    void enqueue(Task task);
    void waitIdle();

    bool running() const { return m_running.load(); }
    u32 workerCount() const { return static_cast<u32>(m_workers.size()); }

private:
    void workerLoop(u32 index);

    std::atomic<bool> m_running { false };
    std::atomic<u32>  m_activeWorkers { 0 };
    std::vector<std::thread> m_workers;
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable m_idleCv;
};

} // namespace Feliss
