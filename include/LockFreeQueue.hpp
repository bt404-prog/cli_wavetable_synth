#pragma once
#include <atomic>
#include <array>
#include <optional>

template <typename T, size_t Capacity>
class LockFreeQueue {
public:
    //UI thread sending the event
    bool push(const T& item) {
        auto current_tail = tail.load(std::memory_order_relaxed);
        auto next_tail = (current_tail + 1) % Capacity;
        
        // Full queue
        if (next_tail == head.load(std::memory_order_acquire)) {
            return false;
        }
        
        buffer[current_tail] = item;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    // Audio thread receiving the event
    std::optional<T> pop() {
        auto current_head = head.load(std::memory_order_relaxed);
        
        // Empty queue
        if (current_head == tail.load(std::memory_order_acquire)) {
            return std::nullopt;
        }
        
        T item = buffer[current_head];
        head.store((current_head + 1) % Capacity, std::memory_order_release);
        return item;
    }

private:
    std::array<T, Capacity> buffer;
    // Alignment to the processor cache line (prevents False Sharing)
    alignas(64) std::atomic<size_t> head{0}; 
    alignas(64) std::atomic<size_t> tail{0}; 
};