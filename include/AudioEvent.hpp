#pragma once
#include <cstdint>

// Event types that interface can send to the engine
enum class EventType {
    NoteOn,
    NoteOff,
    ParameterChange
};

// Event structure
struct AudioEvent {
    EventType type;
    uint8_t note;      // Sending MIDI instead of frequency(0-127)
    uint8_t velocity;  
};





