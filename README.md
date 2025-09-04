# HexCompose

HexCompose is a lightweight Windows utility that provides two advanced keyboard features:

1. **Unicode Hex Input (Compose Mode)**  
   Press **Ctrl+Shift+U** in any application, type a Unicode codepoint in hexadecimal, then press **Space** or **Enter** to insert the character.  
   Example: `Ctrl+Shift+U 20ac Space` → `€`

2. **Caps Lock Accents / Uppercase Accents**  
   When **Caps Lock** is on, pressing accented lowercase letters (produced by your keyboard layout, including dead keys) automatically generates their uppercase equivalents.  
   Example: with Caps Lock ON → `á` → `Á`.

It works by installing a **low-level global keyboard hook** (`WH_KEYBOARD_LL`) on Windows and intercepting keystrokes to implement the features above.

---

## Features

- Global hotkey: **Ctrl+Shift+Pause/Break** → quits the app (panic exit).
- Unicode compose input with live hex buffer (timeout after 5s).
- Accented characters correctly uppercased in any layout.
- Implemented in modern **C++17**, single executable, no dependencies.
- Minimal logging via `OutputDebugStringW`.

---

## Project Structure

```
src/
main.cpp Entry point (wWinMain)
HexComposeApp.* Main app: installs/uninstalls the keyboard hook
hooks/ Hook modules
HookManager.* Dispatches key events to modules
UnicodeComposeHook.* Unicode hex input
CapsAccentsHook.* CapsLock accent uppercasing
util/ Helpers
WinUtils.* WinAPI utilities
Logging.* Minimal debug logging
CMakeLists.txt Build configuration
.devcontainer/ Devcontainer for cross-compilation
```

---

## Building

### Option A: Cross-compile on Linux (Devcontainer with MinGW)

The repository includes a ready-to-use **Devcontainer** with Ubuntu + MinGW-w64 + Wine.  
Inside the devcontainer:

```bash
# Configure with the Windows toolchain
cmake -B build-windows -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=windows-x86_64-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release .

# Build the executable
cmake --build build-windows
```

Result:

```bash
build-windows/HexCompose.exe
```

You can optionally run it under Wine for smoke testing:

```bash
wine build-windows/HexCompose.exe
```

> ⚠️ Note: Keyboard hooks (SetWindowsHookEx) may not behave exactly the same under Wine.
Always test on real Windows before distributing.

--- 

### Option B: Native build on Windows (MSVC or MINGW)

On Windows, you can build with Visual Studio (MSVC) or MinGW directly:

```powershell
# with CMake + MSVC
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

or, with MinGW:

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Distribution 

- The executable links statically to the GCC C++ runtime (`-static-libgcc -static-libstdc++`), so no extra DLLs are needed.
- If static linking of `winpthread` is not available, ship `libwinpthread-1.dll` alongside `HexCompose.exe`.
- Windows system DLLs (`user32.dll`, `kernel32.dll`, etc.) are always present and not included.

## Usage

- Run `HexCompose.exe` (no UI, runs in background).
- Try typing with Caps Lock enabled, or use `Ctrl+Shift+U` to compose Unicode.
- To exit: `Ctrl+Shift+Pause/Break`