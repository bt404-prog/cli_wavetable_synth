#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "AudioEngine.hpp"
#include "AudioEvent.hpp"

int main() {
    using namespace ftxui;

    try {
        AudioEngine engine;
        engine.start();

        auto screen = ScreenInteractive::TerminalOutput();

        const std::map<char, std::uint8_t> keyToMidi = {
            {'a', 60}, // C4
            {'w', 61}, // C#4
            {'s', 62}, // D4
            {'e', 63}, // D#4
            {'d', 64}, // E4
            {'f', 65}, // F4
            {'t', 66}, // F#4
            {'g', 67}, // G4
            {'y', 68}, // G#4
            {'h', 69}, // A4 (440 Hz)
            {'u', 70}, // A#4
            {'j', 71}, // B4
            {'k', 72}  // C5
        };

        std::string lastNote = "None (press a key)";
        float masterGain = 0.2f;

        auto exitButton = Button("Stop & Exit", [&] { screen.Exit(); });

        auto renderer = Renderer(exitButton, [&] {
            return vbox({
                text("VanguardSynth - Lock-Free Audio Engine") | bold | color(Color::Green),
                separator(),
                text("PC Keyboard Piano (Octave 4):"),
                text("  W E   T Y U"),
                text(" A S D F G H J K"),
                text("SPACE: Silence (NoteOff) | - / +: Master Gain"),
                separator(),
                text("Active Note: " + lastNote) | color(Color::Cyan),
                text("Master Gain: " + std::to_string(static_cast<int>(masterGain * 100.0f)) + "%"),
                text("Sample Rate: " + std::to_string(static_cast<int>(engine.sampleRate())) + " Hz"
                     + " | XRUNs: " + std::to_string(engine.xrunCount())
                     + " | Dropped: " + std::to_string(engine.droppedEvents())) | color(Color::GrayLight),
                separator(),
                exitButton->Render()
            }) | border;
        });

        auto component = CatchEvent(renderer, [&](Event event) {
            if (!event.is_character()) {
                return false;
            }

            const std::string& chStr = event.character();
            if (chStr.size() != 1) {
                return false;
            }

            const char rawKey = chStr[0];

            if (rawKey == ' ') {
                if (!engine.pushEvent(AudioEvent::makeNoteOff())) {
                    lastNote = "Queue Full (NoteOff dropped)";
                } else {
                    lastNote = "Muted (Space)";
                }
                return true;
            }

            if (rawKey == '-' || rawKey == '+') {
                masterGain = std::clamp(masterGain + (rawKey == '+' ? 0.05f : -0.05f), 0.0f, 1.0f);
                (void)engine.pushEvent(AudioEvent::makeParamChange(ParamId::MasterGain, masterGain));
                return true;
            }

            const char lowerKey = static_cast<char>(std::tolower(static_cast<unsigned char>(rawKey)));
            if (const auto it = keyToMidi.find(lowerKey); it != keyToMidi.end()) {
                const std::uint8_t note = it->second;
                if (!engine.pushEvent(AudioEvent::makeNoteOn(note, 127))) {
                    lastNote = "Queue Full (NoteOn dropped)";
                } else {
                    lastNote = std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(lowerKey))))
                               + " (MIDI " + std::to_string(note) + ")";
                }
                return true;
            }

            return false;
        });

        screen.Loop(component);
        engine.stop();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}