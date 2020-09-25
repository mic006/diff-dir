/*
Copyright 2020 Michel Palleau

This file is part of diff-dir.

diff-dir is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

diff-dir is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with diff-dir. If not, see <https://www.gnu.org/licenses/>.
*/

/** @file
 *
 * Helpers for concurrency.
 */

#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

/// Queue that can be shared between threads
template <typename T>
class ConcurrentQueue
{
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue()
    {
        close();
    }

    // not copyable
    ConcurrentQueue(const ConcurrentQueue &) = delete;
    ConcurrentQueue &operator=(const ConcurrentQueue &) = delete;

    // not movable
    ConcurrentQueue(ConcurrentQueue &&) noexcept = delete;
    ConcurrentQueue &operator=(ConcurrentQueue &&) noexcept = delete;

    /// Close the queue, free all getters
    void close()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_closed)
                return;
            m_closed = true;
        }
        // wake all getters
        m_condVar.notify_all();
    }

    /// Push one element to the queue
    void push(T &&t)
    {
        // append element to the queue
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.emplace(std::move(t));
        }
        // and notify one getter
        m_condVar.notify_one();
    }

    /** Get one element from the queue.
     * May block until one item is available.
     */
    std::optional<T> get()
    {
        // wait for one element and get it
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condVar.wait(lock, [this]() { return waitPredicate(); });

        if (m_queue.empty())
            return {};

        T t = std::move(m_queue.front());
        m_queue.pop();
        return std::optional{std::move(t)};
    }

private:
    /** Wait predicate for getters.
     * @return whether wait shall be ended.
     */
    bool waitPredicate() const
    {
        return m_closed or not m_queue.empty();
    }

    bool m_closed{false};              ///< state of the queue
    std::mutex m_mutex;                ///< mutex for m_queue and m_condVar
    std::condition_variable m_condVar; ///< condition variable to unlock waiter
    std::queue<T> m_queue;             ///< queue of objects
};