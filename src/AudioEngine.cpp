#include "AudioEngine.hpp"
#include <cmath>
#include <numbers>

AudioEngine::AudioEngine() {
    auto err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(std::string("PortAudio Init Error: ") + Pa_GetErrorText(err));
    }

    float sampleRate = 44100.0f;
    
    // Startujemy z ciszą (faza = 0)
    phaseIncrement = 0.0f;

    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sampleRate,
                               paFramesPerBufferUnspecified, 
                               paCallback, this);

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

void AudioEngine::pushEvent(const AudioEvent& event) {
    eventQueue.push(event);
}

int AudioEngine::paCallback(const void* inputBuffer, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void* userData) {
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;

    auto* engine = static_cast<AudioEngine*>(userData);
    return engine->processAudio(static_cast<float*>(outputBuffer), framesPerBuffer);
}

int AudioEngine::processAudio(float* outputBuffer, unsigned long framesPerBuffer) {
    // 1. Zdejmujemy zdarzenia z kolejki
    while (auto eventOpt = eventQueue.pop()) {
        auto event = eventOpt.value();
        if (event.type == EventType::NoteOn) {
            float frequency = 440.0f * std::pow(2.0f, (event.note - 69.0f) / 12.0f);
            phaseIncrement = (2.0f * std::numbers::pi_v<float> * frequency) / 44100.0f;
        } else if (event.type == EventType::NoteOff) {
            phaseIncrement = 0.0f;
            phase = 0.0f;
        }
    }

    // 2. Wypełniamy bufor audio
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        float sample = 0.0f;
        if (phaseIncrement > 0.0f) {
            sample = std::sin(phase) * 0.1f;
            phase += phaseIncrement;
            if (phase >= 2.0f * std::numbers::pi_v<float>) {
                phase -= 2.0f * std::numbers::pi_v<float>;
            }
        }
        
        *outputBuffer++ = sample; // Lewy kanał
        *outputBuffer++ = sample; // Prawy kanał
    }
    return paContinue;
}