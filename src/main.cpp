#include <iostream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include "AudioEngine.hpp"
#include <map>
#include <string>

int main() {
    using namespace ftxui;

    try {
        AudioEngine engine;
        engine.start();

        auto screen = ScreenInteractive::TerminalOutput();
        
        std::map<std::string, uint8_t> keyToMidi = {
            {"a", 60}, // C4
            {"w", 61}, // C#4
            {"s", 62}, // D4
            {"e", 63}, // D#4
            {"d", 64}, // E4
            {"f", 65}, // F4
            {"t", 66}, // F#4
            {"g", 67}, // G4
            {"y", 68}, // G#4
            {"h", 69}, // A4 (440 Hz)
            {"u", 70}, // A#4
            {"j", 71}, // B4
            {"k", 72}  // C5
        };

        std::string lastNote = "None (Press a button)";

        auto exit_button = Button("Stop and leave", [&] { screen.Exit(); });

        auto renderer = Renderer(exit_button, [&] {
            return vbox({
                text("Vanguard Synth: Comunication test Lock-Free") | bold | color(Color::RedLight),
                separator(),
                text("PC keyboard as a piano(octave 4):"),
                text("  W E   T Y U"),
                text(" A S D F G H J K"),
                separator(),
                text("Press SPACE to stop all sounds"),
                separator(),
                text("Active note: " + lastNote) | color(Color::Cyan),
                separator(),
                exit_button->Render()
            }) | border;
        });

        // Listening for keyboard presses
        auto component = CatchEvent(renderer, [&](Event event) {
            
            if (event.is_character()) {
                std::string key = event.character();
                
                if (key == " ") { // Space is set as killswitch
                    engine.pushEvent({EventType::NoteOff, 0, 0});
                    lastNote = "Wyciszono (Spacja)";
                    return true;
                }
                
                // Checking if pressed key is from our map
                if (keyToMidi.count(key)) {
                    uint8_t note = keyToMidi[key];
                    
                    engine.pushEvent({EventType::NoteOn, note, 127});
                    lastNote = key + " (MIDI: " + std::to_string(note) + ")";
                    return true; 
                }
            }
            return false; 
        });

        //The loop will block the main thread
        screen.Loop(component);
        
        engine.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Blad krytyczny: " << e.what() << "\n";
        return 1;
    }

    return 0;
}


