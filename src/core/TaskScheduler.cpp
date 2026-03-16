#include "core/TaskScheduler.h"

#include "core/Logger.h"

#include <algorithm>

namespace Feliss {

TaskScheduler::~TaskScheduler() {
    stop();
}

bool TaskScheduler::start(u32 workerCount) {
    if (m_running.exchange(true)) {
        return true;
    }

    const u32 hwThreads = std::max(1u, std::thread::hardware_concurrency());
    const u32 desiredWorkers = workerCount > 0 ? workerCount : std::max(1u, hwThreads > 1 ? hwThreads - 1 : 1u);
    m_workers.reserve(desiredWorkers);

    for (u32 index = 0; index < desiredWorkers; ++index) {
        m_workers.emplace_back([this, index]() { workerLoop(index); });
    }

    FLS_INFOF("TaskScheduler", "Started " << desiredWorkers << " worker threads");
    return true;
}

void TaskScheduler::stop() {
    if (!m_running.exchange(false)) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_tasks.empty()) {
            m_tasks.pop();
        }
    }

    m_cv.notify_all();
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_workers.clear();
    m_activeWorkers.store(0);
    FLS_INFO("TaskScheduler", "Stopped worker threads");
}

void TaskScheduler::enqueue(Task task) {
    if (!task) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(std::move(task));
    }
    m_cv.notify_one();
}

void TaskScheduler::waitIdle() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_idleCv.wait(lock, [this]() {
        return m_tasks.empty() && m_activeWorkers.load() == 0;
    });
}

void TaskScheduler::workerLoop(u32 index) {
    FLS_INFOF("TaskScheduler", "Worker " << index << " online");

    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() {
                return !m_running.load() || !m_tasks.empty();
            });

            if (!m_running.load() && m_tasks.empty()) {
                break;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
            ++m_activeWorkers;
        }

        task();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            --m_activeWorkers;
            if (m_tasks.empty() && m_activeWorkers.load() == 0) {
                m_idleCv.notify_all();
            }
        }
    }

    FLS_INFOF("TaskScheduler", "Worker " << index << " offline");
}

} // namespace Feliss
