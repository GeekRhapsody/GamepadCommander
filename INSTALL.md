# Install

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Build All (Linux + AppImage + Windows)

```bash
./scripts/build-all.sh
```

Notes:
- AppImage requires `appimagetool` and `nonsteam` in PATH or pass them via
  `APPIMAGE_CMAKE_ARGS="-DAPPIMAGETOOL=/path/to/appimagetool -DNONSTEAM_TOOL=/path/to/nonsteam"`.
- Windows cross-build needs a MinGW-w64 toolchain and Windows builds of SDL2 (and optional libcurl).
  You can override the toolchain and extra args via:

```bash
WINDOWS_TOOLCHAIN=/path/to/toolchain.cmake \
WINDOWS_CMAKE_ARGS="-DSDL2_DIR=/path/to/sdl2 -DCURL_DIR=/path/to/curl" \
./scripts/build-all.sh
```

## Windows

Install SDL2 (and optional libcurl for FTP support). Example using vcpkg + MSVC:

```powershell
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install sdl2 curl
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

Run `.\build\Release\GamepadCommander.exe`.

For "Add to Steam" on Windows, download `nonsteam.exe` separately and place it next to
`GamepadCommander.exe`.

Archive extraction relies on external `unzip`/`unrar` binaries being available in PATH.

## AppImage

Requires `appimagetool` and `nonsteam` in your PATH (or pass `-DAPPIMAGETOOL=/path/to/appimagetool -DNONSTEAM_TOOL=/path/to/nonsteam`).

```bash
cmake -S . -B build -DBUILD_APPIMAGE=ON -DNONSTEAM_TOOL=/usr/bin/nonsteam
cmake --build build
```

Or build only the AppImage target:

```bash
cmake --build build --target appimage
```
