cmake_minimum_required(VERSION 3.20)

project(HexCompose LANGUAGES CXX VERSION 0.3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Config di build: default a Release se non specificato
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(HexCompose WIN32
    src/main.cpp
    src/HexComposeApp.cpp
    src/hooks/HookManager.cpp
    src/hooks/CapsAccentsHook.cpp
    src/hooks/UnicodeComposeHook.cpp
    src/util/WinUtils.cpp
    src/util/Logging.cpp
)

target_include_directories(HexCompose PRIVATE src)

target_compile_definitions(HexCompose PRIVATE
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN NOMINMAX
    _WIN32_WINNT=0x0601
)

if(MSVC)
    target_compile_options(HexCompose PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(HexCompose PRIVATE -Wall -Wextra -Wpedantic)
endif()

# IMPORTANTE: entrypoint wide (wWinMain) -> -municode
# WIN32 in add_executable aggiunge -mwindows; noi aggiungiamo anche -municode
# Inoltre linkiamo in statico le runtime GCC/STD++ per distribuire 1 solo .exe
# Se -static ti crea problemi con winpthread, commenta la riga -static (lascia le altre due)
target_link_options(HexCompose PRIVATE
    -municode
    -static-libgcc
    -static-libstdc++
    -static
)

# std::thread -> Threads (su MinGW sarà winpthread)
find_package(Threads REQUIRED)

target_link_libraries(HexCompose PRIVATE
    user32
    kernel32
    Threads::Threads
)
