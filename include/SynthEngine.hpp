#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "AudioEvent.hpp"

class SynthEngine {
public:
    explicit SynthEngine(float sampleRate = 48000.0f);

    void setSampleRate(float sampleRate) noexcept;
    void handleEvent(const AudioEvent& event) noexcept;
    void process(float* interleavedStereoOutput, std::size_t frameCount) noexcept;

    [[nodiscard]] float sampleRate() const noexcept { return sampleRate_; }
    [[nodiscard]] float masterGain() const noexcept { return masterGain_; }

private:
    static constexpr std::uint8_t kMaxMidiNote = 127;

    float sampleRate_ = 48000.0f;
    float phase_ = 0.0f;
    float phaseIncrement_ = 0.0f;
    float velocityGain_ = 0.0f;
    float masterGain_ = 0.2f;
    std::array<float, 128> midiFreqTable_{};
};