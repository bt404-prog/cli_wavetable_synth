#pragma once

#include <atomic>
#include <cstdint>
#include <portaudio.h>
#include "AudioEvent.hpp"
#include "SpscRingBuffer.hpp"
#include "SynthEngine.hpp"

class AudioEngine {
public:
    explicit AudioEngine(float requestedSampleRate = 48000.0f,
                         unsigned long framesPerBuffer = 256);
    ~AudioEngine() = default;

    // Non-copyable and non-movable because 'this' is passed as userData to PortAudio.
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&&) = delete;
    AudioEngine& operator=(AudioEngine&&) = delete;

    void start();
    void stop();

    [[nodiscard]] bool pushEvent(const AudioEvent& event) noexcept;

    [[nodiscard]] float sampleRate() const noexcept { return actualSampleRate_; }
    [[nodiscard]] std::uint64_t droppedEvents() const noexcept {
        return droppedEvents_.load(std::memory_order_relaxed);
    }
    [[nodiscard]] std::uint64_t xrunCount() const noexcept {
        return xrunCount_.load(std::memory_order_relaxed);
    }

private:
    struct PortAudioContext {
        PortAudioContext();
        ~PortAudioContext();
        PortAudioContext(const PortAudioContext&) = delete;
        PortAudioContext& operator=(const PortAudioContext&) = delete;
        PortAudioContext(PortAudioContext&&) = delete;
        PortAudioContext& operator=(PortAudioContext&&) = delete;
    };

    struct PortAudioStream {
        PaStream* handle = nullptr;
        PortAudioStream() = default;
        ~PortAudioStream();
        PortAudioStream(const PortAudioStream&) = delete;
        PortAudioStream& operator=(const PortAudioStream&) = delete;
        PortAudioStream(PortAudioStream&&) = delete;
        PortAudioStream& operator=(PortAudioStream&&) = delete;
    };

    static int paCallback(const void* inputBuffer,
                          void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData);

    int processAudio(float* outputBuffer, unsigned long framesPerBuffer) noexcept;

    // Member declaration order enforces safe RAII teardown:
    // stream_ closes first (joining audio thread), then DSP/queue destroy, then paContext_ terminates.
    PortAudioContext paContext_;
    SpscRingBuffer<AudioEvent, 256> eventQueue_;
    SynthEngine synth_;
    std::atomic<std::uint64_t> droppedEvents_{0};
    std::atomic<std::uint64_t> xrunCount_{0};
    float actualSampleRate_ = 48000.0f;
    bool isRunning_ = false;
    PortAudioStream stream_;
};