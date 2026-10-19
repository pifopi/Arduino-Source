# Gamepad input

Connect a USB or Bluetooth gamepad recognized by your operating system, then
focus a console's video display, as for keyboard control. The first available
SDL gamepad is selected automatically. Unplug it to select another connected
gamepad. Discovery can take up to one second. Gamepads follow the existing
controller input locks while automation runs.

Release all buttons, triggers and sticks after changing focus or consoles,
stopping automation, or connecting a controller. Input resumes after the pad is
neutral. Disconnecting a pad releases its controls. Sticks have a 15% radial
deadzone and preserve analog movement outside it. Triggers activate above 50%.

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

Gamepad support is enabled by default in GUI builds on Windows, macOS and Linux.
Qt 6 has no supported Qt Gamepad module, so this feature uses SDL 3. It prefers
an installed `SDL3::SDL3-static` target (3.2 or newer). Otherwise CMake downloads
SDL 3.2.10 from its official release server, verifies its SHA-256, and builds
it statically with only input-related subsystems. No SDL DLL needs deployment.
SDL's license is copied to `SDL3-LICENSE.txt` beside the deployed application.

For offline builds, install SDL's static development package or pass
`-DFETCHCONTENT_SOURCE_DIR_SDL3=/path/to/extracted/SDL3-3.2.10`.
To disable support, pass `-DPA_GAMEPAD=OFF`. Core-only builds do not download or
link SDL. Linux users need permission to access their gamepad's input/hidraw
devices, usually supplied by their distribution's controller udev rules.

## Verification

The standalone tests use SDL's virtual joystick backend and need no physical
controller, Qt, or other application dependencies:

```sh
cmake -S SerialPrograms/Tests/Gamepad -B build/gamepad-tests
cmake --build build/gamepad-tests --config Release
ctest --test-dir build/gamepad-tests -C Release --output-on-failure
```

For hardware verification on each OS, connect a gamepad before and after
launch, check buttons, D-pad diagonals, partial/full stick movement and
triggers, then unplug while holding a button. Check focus loss, console
switching, and automation start/stop while holding controls: release before
resuming input. Native hardware behavior still needs testing on each OS.
