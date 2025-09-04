cmake_minimum_required(VERSION 3.20)

project(HexCompose LANGUAGES CXX VERSION 0.3.0)

# ---- Settings base
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---- Metadati autore/versione per risorse e header
set(HEXCOMPOSE_AUTHOR "frascella.dev")
set(HEXCOMPOSE_VERSION "${PROJECT_VERSION}")

# 0.3.0 -> 0,3,0,0
string(REPLACE "." "," HEXCOMPOSE_FILE_VERSION_COMMA "${PROJECT_VERSION}")
set(HEXCOMPOSE_FILE_VERSION_COMMA "${HEXCOMPOSE_FILE_VERSION_COMMA},0")

# Header di configurazione con macro versione/autore
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/HexComposeVersion.h.in"
  "${GENERATED_DIR}/HexComposeVersion.h"
  @ONLY
)
set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/hexcompose.ico")
file(TO_CMAKE_PATH "${APP_ICON}" APP_ICON_ABSPATH)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/res/app.rc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/app.rc"
  @ONLY
)

# Risorsa VERSIONINFO
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/res/version.rc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
  @ONLY
)

# ---- Target
add_executable(HexCompose WIN32
    src/main.cpp
    src/HexComposeApp.cpp
    src/hooks/HookManager.cpp
    src/hooks/CapsAccentsHook.cpp
    src/hooks/UnicodeComposeHook.cpp
    src/util/WinUtils.cpp
    src/util/Logging.cpp
    src/util/TrayIcon.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
    "${CMAKE_CURRENT_BINARY_DIR}/app.rc"
)

target_include_directories(HexCompose PRIVATE
    src
    res
    "${GENERATED_DIR}"
)

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

# wWinMain wide entrypoint
target_link_options(HexCompose PRIVATE
    -municode
    -static-libgcc
    -static-libstdc++
    -static      # se crea problemi con winpthread, rimuovi questa riga
)

# std::thread -> winpthread
find_package(Threads REQUIRED)

target_link_libraries(HexCompose PRIVATE
    user32
    kernel32
    shell32
    Threads::Threads
)
