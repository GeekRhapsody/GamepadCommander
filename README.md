# GamepadCommander

Gamepad-first, two-pane file manager inspired by Total Commander. Uses SDL2 for controller input and rendering, matching the core controller/UI stack used by the emulationstation-de-mod project.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/GamepadCommander
```

## Controls (Gamepad)

- D-Pad Up/Down: Move selection
- A: Enter directory
- B: Go to parent directory
- X: Open actions menu on a file
- L1 / R1: Switch active pane
- Select: Open app menu (Settings, Connect to FTP, Quit)
- Start: Confirm rename
- Y: Clear rename buffer

Rename modal:
- Left/Right: Choose character
- A: Add character
- X: Backspace
- B: Cancel

## Controls (Keyboard)

- Up/Down: Move selection
- Enter: Enter directory
- Backspace: Go to parent directory
- Tab: Switch active pane
- X: Open actions menu on a file
- Esc: Open app menu / close modals
- Typing: Rename input

## App Menu

- Settings: Configure FTP host/port/user/password, UI scale, and show hidden files
- Connect to FTP: Switch the active pane to the configured FTP site
- Quit: Exit with confirmation

## FTP

- File and directory copy/move between FTP and local panes, plus delete/rename on FTP, require libcurl at build time.
