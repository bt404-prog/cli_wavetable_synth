cmake_minimum_required(VERSION 3.24)
project(VanguardSynth VERSION 0.1.0 LANGUAGES CXX)

# Wymuszamy nowoczesny standard C++20 (potrzebny do atomików i konceptów)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Definiujemy rygorystyczne flagi, ale NIE aplikujemy ich globalnie
if(MSVC)
    set(MY_STRICT_WARNINGS /W4 /WX /permissive- /Zc:__cplusplus)
else()
    set(MY_STRICT_WARNINGS -Wall -Wextra -Wpedantic -Werror)
endif()

# Automatyczne pobieranie zależności
include(FetchContent)

# 1. Pobieranie interfejsu graficznego do terminala (FTXUI)
message(STATUS "Pobieranie FTXUI...")
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG main
)
FetchContent_MakeAvailable(ftxui)

# 2. Pobieranie silnika audio (PortAudio)
# Wyłączamy zbędne testy z zewnętrznej biblioteki, żeby przyspieszyć kompilację
set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
message(STATUS "Pobieranie PortAudio...")
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG master
)
FetchContent_MakeAvailable(portaudio)

# Główny cel aplikacji
add_executable(VanguardSynth src/main.cpp src/AudioEngine.cpp)
target_include_directories(VanguardSynth PRIVATE include)

# Linkowanie bibliotek do naszego pliku binarnego
target_link_libraries(VanguardSynth PRIVATE ftxui::screen ftxui::dom ftxui::component portaudio)

# Aplikujemy rygorystyczne flagi TYLKO do naszego własnego kodu (VanguardSynth)
target_compile_options(VanguardSynth PRIVATE ${MY_STRICT_WARNINGS})

# Kopiowanie zależności DLL obok pliku wykonywalnego na systemie Windows
if(WIN32)
    add_custom_command(TARGET VanguardSynth POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:portaudio>"
        "$<TARGET_FILE_DIR:VanguardSynth>"
    )
endif()