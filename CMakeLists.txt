cmake_minimum_required(VERSION 3.20)

project(HexCompose LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Eseguibile WIN32 (niente console window)
add_executable(HexCompose WIN32
    src/main.cpp
    src/HexComposeApp.cpp
    src/hooks/HookManager.cpp
    src/hooks/CapsAccentsHook.cpp
    src/util/WinUtils.cpp
    src/util/Logging.cpp
)

target_include_directories(HexCompose PRIVATE src)

target_compile_definitions(HexCompose PRIVATE
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN NOMINMAX
)

# Warning level e UTF-8 su MSVC
if(MSVC)
    target_compile_options(HexCompose PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(HexCompose PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(HexCompose PRIVATE
    user32
    kernel32
)
