#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>
#include <type_traits>

#if defined(__APPLE__) && defined(__aarch64__)
inline constexpr std::size_t kCacheLineSize = 128;
#else
inline constexpr std::size_t kCacheLineSize = 64;
#endif

// Lock-free Single-Producer Single-Consumer (SPSC) ring buffer.
// Note: One slot distinguishes full from empty, so usable capacity is (Capacity - 1).
// Calling push() from multiple threads or pop() from multiple threads is undefined behavior.
template <typename T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "SpscRingBuffer Capacity must be a power of two >= 2.");
    static_assert(std::is_trivially_copyable_v<T>,
                  "SpscRingBuffer elements must be trivially copyable for real-time safety.");
    static_assert(std::atomic<std::size_t>::is_always_lock_free,
                  "std::atomic<std::size_t> must be lock-free on this platform.");

public:
    SpscRingBuffer() = default;
    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;
    SpscRingBuffer(SpscRingBuffer&&) = delete;
    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    [[nodiscard]] bool push(const T& item) noexcept {
        const std::size_t currentTail = tail_.load(std::memory_order_relaxed);
        const std::size_t nextTail = (currentTail + 1) & kMask;

        if (nextTail == head_.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }

        buffer_[currentTail] = item;
        tail_.store(nextTail, std::memory_order_release);
        return true;
    }

    [[nodiscard]] std::optional<T> pop() noexcept {
        const std::size_t currentHead = head_.load(std::memory_order_relaxed);

        if (currentHead == tail_.load(std::memory_order_acquire)) {
            return std::nullopt; // Buffer empty
        }

        const T item = buffer_[currentHead];
        head_.store((currentHead + 1) & kMask, std::memory_order_release);
        return item;
    }

private:
    static constexpr std::size_t kMask = Capacity - 1;

    std::array<T, Capacity> buffer_{};
    alignas(kCacheLineSize) std::atomic<std::size_t> head_{0};
    alignas(kCacheLineSize) std::atomic<std::size_t> tail_{0};
};