# GamepadCommander

Gamepad-first, two-pane file manager inspired by Total Commander.

## Build

```bash
cmake -S . -B build
cmake --build build
```

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

## Run

```bash
./build/GamepadCommander
```

## Controls (Gamepad)

- D-Pad Up/Down: Move selection
- A: Enter directory
- B: Go to parent directory
- X: Open actions menu on a file (includes "Add to Steam" for .exe)
- L1 / R1: Switch active pane
- Select: Open app menu (Settings, Connect to FTP, Quit)
- Start: Confirm rename

## Controls (Keyboard)

- Up/Down: Move selection
- Enter: Enter directory
- Backspace: Go to parent directory
- Tab: Switch active pane
- X: Open actions menu on a file (includes "Add to Steam" for .exe)
- Esc: Open app menu / close modals

## App Menu

- Settings: Configure FTP host/port/user/password, Steam launch options, UI scale, and show hidden files
- Connect to FTP: Switch the active pane to the configured FTP site
- Quit: Exit with confirmation
