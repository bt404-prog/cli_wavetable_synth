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

        std::string lastNote = "Brak (Wcisnij klawisz)";

        auto exit_button = Button("Zatrzymaj i Wyjdz", [&] { screen.Exit(); });

        auto renderer = Renderer(exit_button, [&] {
            return vbox({
                text("Vanguard Synth: Test Komunikacji Lock-Free") | bold | color(Color::RedLight),
                separator(),
                text("Klawiatura PC jako pianino (oktawa 4):"),
                text("  W E   T Y U"),
                text(" A S D F G H J K"),
                separator(),
                text("Wcisnij SPACJE, aby wyciszyc dzwiek (NoteOff)."),
                separator(),
                text("Aktywna nuta: " + lastNote) | color(Color::Cyan),
                separator(),
                exit_button->Render()
            }) | border;
        });

        // Nasłuchiwanie na wciśnięcia klawiatury
        auto component = CatchEvent(renderer, [&](Event event) {
            // Reagujemy tylko na znaki z klawiatury
            if (event.is_character()) {
                std::string key = event.character();
                
                if (key == " ") { // Spacja to nasz Killswitch
                    engine.pushEvent({EventType::NoteOff, 0, 0});
                    lastNote = "Wyciszono (Spacja)";
                    return true;
                }
                
                // Jeśli wciśnięto klawisz z naszej mapy
                if (keyToMidi.count(key)) {
                    uint8_t note = keyToMidi[key];
                    // WYSYŁAMY PAKIET DO STREFY CZERWONEJ
                    engine.pushEvent({EventType::NoteOn, note, 127});
                    lastNote = key + " (MIDI: " + std::to_string(note) + ")";
                    return true; 
                }
            }
            return false; // Zdarzenie nie zostało obsłużone, przekaż dalej (np. kliknięcie myszką w przycisk)
        });

        // Pętla zablokuje główny wątek, a callback audio pracuje w tle
        screen.Loop(component);
        
        engine.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Blad krytyczny: " << e.what() << "\n";
        return 1;
    }

    return 0;
}