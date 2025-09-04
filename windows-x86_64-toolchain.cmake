# windows-x86_64-toolchain.cmake
# Cross-compile verso Windows 64-bit con MinGW-w64

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compilatori MinGW-w64
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Dove trovare librerie/include del target
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Cerca librerie/include solo nel root path del target
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Nota: NON aggiungiamo qui flag di link; li gestiamo nel CMakeLists per evitare duplicazioni.
