#include "AudioEngine.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>

AudioEngine::PortAudioContext::PortAudioContext() {
    const PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(std::string("PortAudio Init Error: ") + Pa_GetErrorText(err));
    }
}

AudioEngine::PortAudioContext::~PortAudioContext() {
    Pa_Terminate();
}

AudioEngine::PortAudioStream::~PortAudioStream() {
    if (handle != nullptr) {
        if (Pa_IsStreamActive(handle) == 1) {
            Pa_StopStream(handle);
        }
        Pa_CloseStream(handle);
    }
}

AudioEngine::AudioEngine(float requestedSampleRate, unsigned long framesPerBuffer)
    : synth_(requestedSampleRate), actualSampleRate_(requestedSampleRate) {
    const PaError err = Pa_OpenDefaultStream(
        &stream_.handle,
        0,
        2,
        paFloat32,
        static_cast<double>(requestedSampleRate),
        framesPerBuffer,
        paCallback,
        this);

    if (err != paNoError) {
        throw std::runtime_error(std::string("PortAudio Stream Error: ") + Pa_GetErrorText(err));
    }

    const PaStreamInfo* info = Pa_GetStreamInfo(stream_.handle);
    if (info != nullptr && info->sampleRate > 0.0) {
        actualSampleRate_ = static_cast<float>(info->sampleRate);
    }
    synth_.setSampleRate(actualSampleRate_);
}

void AudioEngine::start() {
    if (stream_.handle == nullptr || isRunning_) {
        return;
    }

    const PaError err = Pa_StartStream(stream_.handle);
    if (err != paNoError) {
        throw std::runtime_error(std::string("PortAudio Start Error: ") + Pa_GetErrorText(err));
    }
    isRunning_ = true;
}

void AudioEngine::stop() {
    if (stream_.handle == nullptr || !isRunning_) {
        return;
    }

    const PaError err = Pa_StopStream(stream_.handle);
    if (err != paNoError) {
        throw std::runtime_error(std::string("PortAudio Stop Error: ") + Pa_GetErrorText(err));
    }
    isRunning_ = false;
}

bool AudioEngine::pushEvent(const AudioEvent& event) noexcept {
    if (!eventQueue_.push(event)) {
        droppedEvents_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    return true;
}

int AudioEngine::paCallback(const void* inputBuffer,
                            void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void* userData) {
    (void)inputBuffer;
    (void)timeInfo;

    auto* engine = static_cast<AudioEngine*>(userData);
    if ((statusFlags & paOutputUnderflow) != 0) {
        engine->xrunCount_.fetch_add(1, std::memory_order_relaxed);
    }

    return engine->processAudio(static_cast<float*>(outputBuffer), framesPerBuffer);
}

int AudioEngine::processAudio(float* outputBuffer, unsigned long framesPerBuffer) noexcept {
    while (const auto eventOpt = eventQueue_.pop()) {
        synth_.handleEvent(*eventOpt);
    }

    synth_.process(outputBuffer, static_cast<std::size_t>(framesPerBuffer));
    return paContinue;
}