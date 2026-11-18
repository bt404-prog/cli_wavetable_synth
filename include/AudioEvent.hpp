#pragma once
#include <cstdint>

// Typy zdarzeń, które interfejs może wysłać do silnika
enum class EventType {
    NoteOn,
    NoteOff,
    ParameterChange
};

// Struktura zdarzenia - musi być trywialnie kopiowalna (TriviallyCopyable)
struct AudioEvent {
    EventType type;
    uint8_t note;      // Zamiast częstotliwości, wysyłamy numer nuty MIDI (0-127)
    uint8_t velocity;  // Siła uderzenia (0-127)
};