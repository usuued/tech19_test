#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

template<typename T>
class ThreadSafeQueue {
public:
    explicit ThreadSafeQueue(size_t max_size = 2048)
        : max_size_(max_size), shutdown_(false) {}

    // Push item to queue (blocks if queue is full)
    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until space is available or shutdown
        cv_push_.wait(lock, [this] {
            return queue_.size() < max_size_ || shutdown_;
        });

        if (shutdown_) {
            return;
        }

        queue_.push(std::move(item));
        cv_pop_.notify_one();
    }

    // Pop item from queue (blocks if queue is empty)
    // Returns false if shutdown and queue is empty
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until data is available or shutdown
        cv_pop_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
        });

        if (shutdown_ && queue_.empty()) {
            return false;
        }

        item = std::move(queue_.front());
        queue_.pop();
        cv_push_.notify_one();
        return true;
    }

    // Signal shutdown to all waiting threads
    void shutdown() {
        shutdown_ = true;
        cv_push_.notify_all();
        cv_pop_.notify_all();
    }

    // Check if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // Get current size
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_push_;  // Notified when space available
    std::condition_variable cv_pop_;   // Notified when data available
    const size_t max_size_;
    std::atomic<bool> shutdown_;
};

#endif // THREAD_SAFE_QUEUE_H
