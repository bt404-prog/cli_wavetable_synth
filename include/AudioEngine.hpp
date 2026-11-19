#pragma once
#include <portaudio.h>
#include <stdexcept>
#include <string>
#include "LockFreeQueue.hpp"
#include "AudioEvent.hpp"

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    void start();
    void stop();
    void pushEvent(const AudioEvent& event);

private:
    PaStream* stream = nullptr;
    
    float phase = 0.0f;
    float phaseIncrement = 0.0f;

    LockFreeQueue<AudioEvent, 256> eventQueue;

    static int paCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData);

    int processAudio(float* outputBuffer, unsigned long framesPerBuffer);
};







