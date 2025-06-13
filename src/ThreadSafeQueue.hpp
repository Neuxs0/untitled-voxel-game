#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

// A thread-safe queue for producer-consumer patterns.
template <typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond_var;

public:
    void push(T value)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
        lock.unlock();
        m_cond_var.notify_one();
    }

    bool try_pop(T &out)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty())
        {
            return false;
        }
        out = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    bool wait_and_pop(T &out, const std::atomic<bool> &shutdown_flag)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond_var.wait(lock, [this, &shutdown_flag]
                        { return !m_queue.empty() || shutdown_flag.load(); });

        if (shutdown_flag.load() && m_queue.empty())
        {
            return false;
        }

        out = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void notify_all()
    {
        m_cond_var.notify_all();
    }
};
