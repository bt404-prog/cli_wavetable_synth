#pragma once
#include <portaudio.h>
#include <stdexcept>
#include <string>

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    // Blokujemy kopiowanie klasy - karta dźwiękowa może być otwarta tylko raz
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    void start();
    void stop();

private:
    PaStream* stream = nullptr;
    
    // Zmienne do wygenerowania testowej fali (proof of life)
    float phase = 0.0f;
    float phaseIncrement = 0.0f;

    // Statyczny callback w stylu C, wymagany przez PortAudio
    static int paCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData);

    // Metoda wewnątrz obiektu, do której przekażemy sterowanie z callbacku
    int processAudio(float* outputBuffer, unsigned long framesPerBuffer);
};