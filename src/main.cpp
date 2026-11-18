#include <iostream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "AudioEngine.hpp"

int main() {
    using namespace ftxui;

    try {
        // Inicjalizacja naszej nowej klasy uderza prosto w sprzęt
        AudioEngine engine;
        engine.start();

        // Prosty UI, który zablokuje główny wątek, podczas gdy wątek audio gra w tle
        auto screen = ScreenInteractive::TerminalOutput();
        
        auto exit_button = Button("Zatrzymaj i Wyjdz", [&] { screen.Exit(); });
        
        auto document = Renderer(exit_button, [&] {
            return vbox({
                text("Vanguard Synth: Odtwarzanie...") | bold | color(Color::Green),
                separator(),
                text("Slyszysz czyste 440 Hz (A4)."),
                text("Watek Real-Time DSP dziala w tle bez zacinania UI."),
                separator(),
                exit_button->Render()
            }) | border;
        });

        // Loop - program stoi w tym miejscu, dopóki nie klikniesz przycisku
        screen.Loop(document);
        
        engine.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Blad krytyczny: " << e.what() << "\n";
        return 1;
    }

    return 0;
}