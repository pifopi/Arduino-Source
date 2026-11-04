# Gamepad input

Connect a USB or Bluetooth gamepad recognized by your operating system, then
focus a console's video display, as for keyboard control. The first available
SDL gamepad is selected automatically. Unplug it to select another connected
gamepad. Discovery can take up to one second. Gamepads follow the existing
controller input locks while automation runs.

Release all buttons, triggers and sticks after changing focus or consoles,
stopping automation, or connecting a controller. Input resumes after the pad is
neutral. Disconnecting a pad releases its controls. Sticks have a 15% radial
deadzone and preserve analog movement outside it. Triggers activate above 20%.

Button mapping follows Nintendo physical positions: bottom = B, right = A,
left = Y, top = X. Thus Xbox A sends Nintendo B, and Xbox B sends Nintendo A.
Shoulders and triggers send L/R and ZL/ZR, Back/Select sends Minus,
Start sends Plus, Guide sends Home, and SDL's miscellaneous button sends
Capture where available. Stick clicks and D-pad map directly. Both sticks work
on Pro Controllers. A left Joy-Con uses the left stick and D-pad; a right
Joy-Con uses the right stick and face buttons. OS-reserved buttons may be
unavailable. Keyboard mappings do not change the gamepad mapping. Use one input
device at a time; keyboard and gamepad commands replace each other's state.

## Building

Gamepad support is disabled by default on Windows, macOS and Linux. Pass
`-DPA_GAMEPAD=ON` when configuring CMake to enable it, or `-DPA_GAMEPAD=OFF`
to disable it in an existing build. Desktop CI explicitly enables gamepad
support.

Windows builds use SDL 3.4.20 headers in `3rdParty/SDL3` and the x64 import
library and DLL in `3rdPartyBinaries`,
following the other bundled dependencies. The existing DLL deployment step
copies `SDL3.dll` beside the application.

The bundled files come from the official
[SDL3-devel-3.4.20-VC.zip](https://www.libsdl.org/release/SDL3-devel-3.4.20-VC.zip)
(SHA-256: `1ec3009655ee37b1f73e52b3b120acc5afd8abaf9532f3303b1bf1dfe91b5969`).

When gamepad support is enabled, macOS and Linux builds use the packaged
SDL 3.2 or newer discovered through its CMake package. Install `libsdl3-dev` on
Ubuntu 26.04 or newer, or run `brew install sdl3` on macOS. CI installs those
packages directly; SDL is not built from source. The packaged headers take
precedence over the bundled Windows headers. On macOS, package symlinks are
resolved to keep SDL's include directory separate from unrelated Homebrew
headers.

Builds with gamepad support disabled and core-only builds do not search for
or link SDL. Linux users need permission to access gamepad input/hidraw devices,
usually supplied by their distribution's controller udev rules.

## Verification

For hardware verification on each OS, connect a gamepad before and after
launch, check buttons, D-pad diagonals, partial/full stick movement and
triggers, then unplug while holding a button. Check focus loss, console
switching, and automation start/stop while holding controls: release before
resuming input. Native hardware behavior still needs testing on each OS.
