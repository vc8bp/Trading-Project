#pragma once

#include <map>
#include <set>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <unistd.h>

template <typename T>
class ThreadSafeMap {
public:
    void update(int key, const T& value);
    T get(int key);
    T getBatchedFeed();
    std::string printAll();

private:
    std::set<int> updatedKeys_;
    std::map<int, T> dataMap_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};

template <typename T>
void ThreadSafeMap<T>::update(int key, const T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    dataMap_[key] = value;
    updatedKeys_.insert(key);
    condition_.notify_one();
}

template <typename T>
T ThreadSafeMap<T>::get(int key) {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this, key] { return dataMap_.find(key) != dataMap_.end(); });
    T value = dataMap_[key];
    dataMap_.erase(key);
    return value;
}

template <typename T>
T ThreadSafeMap<T>::getBatchedFeed() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::set<int> keys = updatedKeys_;

    T finalString;

    for(int key: keys){
        T s = dataMap_[key];
        finalString += (finalString.empty() ? "" : ",") + s;
    }

    keys.clear();  
    usleep(100);
    return "[" + finalString + "]"; 
}

template <typename T>
std::string ThreadSafeMap<T>::printAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::stringstream result;

    for (const auto& entry : dataMap_) {
        result << "Key: " << entry.first << ", Value: " << entry.second << '\n';
    }

    return result.str();
}
template class ThreadSafeMap<std::string>;
