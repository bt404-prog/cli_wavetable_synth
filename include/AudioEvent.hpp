#pragma once

#include <cstdint>
#include <type_traits>

enum class EventType : std::uint8_t {
    NoteOn,
    NoteOff,
    ParameterChange
};

enum class ParamId : std::uint8_t {
    MasterGain
};

struct NoteData {
    std::uint8_t note = 0;      // MIDI note (0-127)
    std::uint8_t velocity = 0;  // MIDI velocity (0-127)
};

struct ParamData {
    ParamId id = ParamId::MasterGain;
    float value = 0.0f;
};

struct AudioEvent {
    EventType type = EventType::NoteOff;

    union Payload {
        NoteData note;
        ParamData param;

        constexpr Payload() noexcept : note{} {}
        constexpr explicit Payload(NoteData n) noexcept : note(n) {}
        constexpr explicit Payload(ParamData p) noexcept : param(p) {}
    } data{};

    [[nodiscard]] static constexpr AudioEvent makeNoteOn(std::uint8_t note, std::uint8_t velocity) noexcept {
        return AudioEvent{EventType::NoteOn, Payload{NoteData{note, velocity}}};
    }

    [[nodiscard]] static constexpr AudioEvent makeNoteOff(std::uint8_t note = 0) noexcept {
        return AudioEvent{EventType::NoteOff, Payload{NoteData{note, 0}}};
    }

    [[nodiscard]] static constexpr AudioEvent makeParamChange(ParamId id, float value) noexcept {
        return AudioEvent{EventType::ParameterChange, Payload{ParamData{id, value}}};
    }
};

static_assert(std::is_trivially_copyable_v<AudioEvent>,
              "AudioEvent must remain trivially copyable for lock-free transport.");