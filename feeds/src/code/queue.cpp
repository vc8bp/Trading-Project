#include "../includes/queue.h"



template <typename T>
void ThreadSafeQueue<T>::enqueue(const T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(value);
    if(queue_.size() > 100){
        queue_.pop();
    }
    condition_.notify_one();
}

template <typename T>
int ThreadSafeQueue<T>::size(){
    return queue_.size();
}

template <typename T>
T ThreadSafeQueue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return !queue_.empty(); });
    T value = queue_.front();
    queue_.pop();
    return value;
}

// Explicit instantiation to avoid linking issues
template class ThreadSafeQueue<std::string>;
