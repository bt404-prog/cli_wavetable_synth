# VanguardSynth

A Real-Time Wavetable Synthesizer with a Terminal User Interface (TUI), built with Modern C++20. 

This project demonstrates strict multi-threading architecture, separating the real-time DSP (Digital Signal Processing) audio thread from the Main/UI thread using a custom Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer.

## Core Architecture
* **Red Zone (Audio/DSP Thread):** Powered by **PortAudio**. Runs strictly in real-time. No mutexes, no dynamic memory allocation, and no system calls to prevent audio dropouts.
* **Green Zone (Main/UI Thread):** Powered by **FTXUI**. Renders a reactive terminal interface and captures user keyboard inputs.
* **The Bridge (Lock-Free Queue):** Thread-safe communication utilizing `std::atomic` operations, explicit memory barriers (`std::memory_order_acquire` / `release`), and strict cache-line alignment (`alignas(64)`) to prevent false sharing.

## Current State (Phase 1 & 2)
* [x] Modern CMake build system configuration (Auto-fetching dependencies).
* [x] Strict compiler flags enabled (`/WX` - Warnings as Errors).
* [x] RAII-compliant PortAudio engine wrapper.
* [x] Lock-Free Event Queue implementation.
* [ ] Wavetable oscillators and envelope generators (In Progress).
* [ ] Terminal keyboard routing to DSP engine (In Progress).

## Dependencies
All dependencies are automatically fetched and built from source via CMake `FetchContent`:
* [PortAudio](http://www.portaudio.com/) - Cross-platform Audio I/O library.
* [FTXUI](https://github.com/ArthurSonzogni/FTXUI) - Functional Terminal (X) User Interface.

## Build Instructions (Windows / MSVC)
This project requires Visual Studio Build Tools (MSVC) and CMake.

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release