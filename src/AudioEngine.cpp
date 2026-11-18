#include "AudioEngine.hpp"
#include <cmath>
#include <numbers>

AudioEngine::AudioEngine() {
    auto err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(std::string("PortAudio Init Error: ") + Pa_GetErrorText(err));
    }

    // 44100 próbek na sekundę, częstotliwość 440Hz
    float sampleRate = 44100.0f;
    float frequency = 440.0f;
    phaseIncrement = (2.0f * std::numbers::pi_v<float> * frequency) / sampleRate;

    // Otwieramy domyślny strumień wyjściowy: 0 wejść, 2 wyjścia (Stereo), 32-bit float
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sampleRate,
                               paFramesPerBufferUnspecified, 
                               paCallback, this); // 'this' leci jako userData

    if (err != paNoError) {
        throw std::runtime_error(std::string("PortAudio Stream Error: ") + Pa_GetErrorText(err));
    }
}

AudioEngine::~AudioEngine() {
    if (stream) {
        Pa_CloseStream(stream);
    }
    Pa_Terminate();
}

void AudioEngine::start() {
    auto err = Pa_StartStream(stream);
    if (err != paNoError) throw std::runtime_error(Pa_GetErrorText(err));
}

void AudioEngine::stop() {
    auto err = Pa_StopStream(stream);
    if (err != paNoError) throw std::runtime_error(Pa_GetErrorText(err));
}

int AudioEngine::paCallback(const void* inputBuffer, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void* userData) {
    // Uciszamy ostrzeżenia o nieużywanych parametrach
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;

    // Rzutujemy wskaźnik C na naszą klasę C++
    auto* engine = static_cast<AudioEngine*>(userData);
    return engine->processAudio(static_cast<float*>(outputBuffer), framesPerBuffer);
}

int AudioEngine::processAudio(float* outputBuffer, unsigned long framesPerBuffer) {
    // Ta pętla wykonuje się w wątku Real-Time.
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        // Generujemy próbkę i ściszamy do 10% (0.1f)
        float sample = std::sin(phase) * 0.1f; 
        
        *outputBuffer++ = sample; // Kanał Lewy
        *outputBuffer++ = sample; // Kanał Prawy
        
        phase += phaseIncrement;
        if (phase >= 2.0f * std::numbers::pi_v<float>) {
            phase -= 2.0f * std::numbers::pi_v<float>;
        }
    }
    return paContinue;
}