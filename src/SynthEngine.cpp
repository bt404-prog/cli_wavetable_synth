#include "SynthEngine.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace {
constexpr float kTwoPi = 2.0f * std::numbers::pi_v<float>;
}

SynthEngine::SynthEngine(float sampleRate) {
    if (sampleRate <= 0.0f) {
        throw std::invalid_argument("Sample rate must be positive.");
    }
    sampleRate_ = sampleRate;

    // Precompute MIDI note (0-127) frequencies once off the audio thread.
    for (std::size_t i = 0; i < midiFreqTable_.size(); ++i) {
        midiFreqTable_[i] = 440.0f * std::pow(2.0f, (static_cast<float>(i) - 69.0f) / 12.0f);
    }
}

void SynthEngine::setSampleRate(float sampleRate) noexcept {
    if (sampleRate > 0.0f) {
        sampleRate_ = sampleRate;
    }
}

void SynthEngine::handleEvent(const AudioEvent& event) noexcept {
    switch (event.type) {
        case EventType::NoteOn: {
            const std::uint8_t note = event.data.note.note;
            const std::uint8_t velocity = std::min(event.data.note.velocity, kMaxMidiNote);

            // Standard MIDI convention: NoteOn with velocity 0 acts as NoteOff.
            if (velocity == 0) {
                phaseIncrement_ = 0.0f;
                velocityGain_ = 0.0f;
                phase_ = 0.0f;
                break;
            }

            if (note > kMaxMidiNote) {
                break;
            }

            const float frequency = midiFreqTable_[note];
            const float nyquist = sampleRate_ * 0.5f;
            if (frequency >= nyquist) {
                phaseIncrement_ = 0.0f;
                break;
            }

            phaseIncrement_ = (kTwoPi * frequency) / sampleRate_;
            velocityGain_ = static_cast<float>(velocity) / 127.0f;
            break;
        }
        case EventType::NoteOff: {
            phaseIncrement_ = 0.0f;
            velocityGain_ = 0.0f;
            phase_ = 0.0f;
            break;
        }
        case EventType::ParameterChange: {
            switch (event.data.param.id) {
                case ParamId::MasterGain:
                    masterGain_ = std::clamp(event.data.param.value, 0.0f, 1.0f);
                    break;
            }
            break;
        }
    }
}

void SynthEngine::process(float* interleavedStereoOutput, std::size_t frameCount) noexcept {
    if (interleavedStereoOutput == nullptr) {
        return;
    }

    const float gain = masterGain_ * velocityGain_;

    for (std::size_t i = 0; i < frameCount; ++i) {
        float sample = 0.0f;
        if (phaseIncrement_ > 0.0f) {
            sample = std::sin(phase_) * gain;
            phase_ += phaseIncrement_;
            // Safe single subtraction because frequency < Nyquist guarantees phaseIncrement_ < pi.
            if (phase_ >= kTwoPi) {
                phase_ -= kTwoPi;
            }
        }

        *interleavedStereoOutput++ = sample; // Left
        *interleavedStereoOutput++ = sample; // Right
    }
}