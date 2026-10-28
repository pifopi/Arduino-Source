# SFML gamepad input

Connect a USB or Bluetooth controller recognized by your operating system, then
focus a console's video display, as for keyboard control. The first connected
SFML joystick is selected automatically. Unplug it to select another one, or
choose a fixed slot in the mapping file described below. SFML refreshes device
connections while polling, without creating a second window or event loop.

The controller follows the existing focus and automation input locks. Release
all controls after connecting, switching consoles, or stopping automation.
Disconnecting releases its inputs. Both sticks retain analog movement, with a
15% radial deadzone for drift. Keyboard and gamepad commands replace each
other's state; use one input device at a time.

Button positions follow Nintendo: bottom = B, right = A, left = Y, top = X.
Shoulders send L/R, triggers send ZL/ZR, Back sends Minus and Start sends Plus.
Stick clicks and the D-pad map directly. Both sticks work on Pro Controllers.
A left Joy-Con uses the left stick and D-pad; a right Joy-Con uses the right
stick and face buttons. Guide and Capture can be mapped when exposed by the OS.

## Mapping your controller

SFML reports raw buttons and axes, whose layout varies by controller, driver
and OS. Unlike SDL's gamepad API, it does not standardize their physical layout.
The application creates `SFML-Gamepad.json` in its settings folder on first
launch. Windows defaults target common Xbox/DirectInput layouts. Linux defaults
target the common xpad layout. macOS starts with the Windows-style defaults;
verify and adjust them for your device. One mapping applies to the selected pad.

Close the application, edit the file, and restart to apply changes. Invalid
mappings disable gamepad input and log the reason; they do not prevent startup.
Keyboard mappings are independent. The file contains:

- `joystick`: `-1` for the first connected pad, or SFML slot `0` through `7`.
- `buttons`: physical names (`South`, `East`, `West`, `North`, `Back`, `Guide`,
  `Start`, `LeftStick`, `RightStick`, `LeftShoulder`, `RightShoulder`, `Up`,
  `Down`, `Left`, `Right`, `Capture`) mapped to raw indices `0` through `31`.
  `-1` disables a binding. Direction buttons can supplement a POV hat.
- `left_x`, `left_y`, `right_x`, `right_y`, `pov_x`, `pov_y`: objects with
  `axis` set to `X`, `Y`, `Z`, `R`, `U`, `V`, `PovX`, `PovY` or `None`, and
  `invert` set to `true` or `false`. Sticks expect positive X right and positive
  Y down. POV expects positive X right and positive Y up.
- `left_trigger`, `right_trigger`: axis objects with additional `rest`,
  `threshold`, and `button` fields. A trigger activates when its axis value,
  after optional inversion, minus `rest` exceeds `threshold`. A mapped raw
  `button` can also activate it (`-1` disables this). SFML axes range -100..100.
- `log_raw_input`: set to `true` temporarily to log pressed button indices and
  supported axis values twice per second. Restart after changing this setting.
  The connection log also shows the device name, vendor/product IDs and slot.

Common Windows Xbox drivers combine both triggers on Z: left uses Z with
`rest: 0`, `threshold: 50`; right uses inverted Z with the same values. The
combined axis cannot distinguish both triggers held equally. For separate
trigger axes resting at -100, use `rest: -100`, `threshold: 100` without inversion.
For digital triggers, use `axis: "None"` and set `button` to the desired index.

Example override (other bindings retain platform defaults):

```json
{
    "joystick": -1,
    "buttons": {"South": 1, "East": 2, "West": 0, "North": 3},
    "right_x": {"axis": "Z", "invert": false},
    "right_y": {"axis": "R", "invert": false},
    "left_trigger": {"axis": "None", "button": 6},
    "right_trigger": {"axis": "None", "button": 7}
}
```

## Building

Support is enabled by default in GUI builds. CMake prefers installed static
SFML 3 Window/System libraries. Otherwise it downloads the official SFML 3.0.2
source archive, verifies SHA-256, and builds only Window/System statically.
No SFML DLL deployment is needed. `SFML-LICENSE.txt` is copied beside the deployed
application. Mapping files reuse the repository's existing nlohmann JSON library.

For offline builds, install static SFML development libraries or pass
`-DFETCHCONTENT_SOURCE_DIR_SFML=/path/to/SFML-3.0.2`. Disable this feature with
`-DPA_GAMEPAD=OFF`. Core-only builds do not download or link SFML. On Linux,
SFML Window needs OpenGL, X11 (Xrandr, Xcursor, Xi) and udev development packages,
and users need permission to access joystick devices. macOS uses SFML's native
backend. Native hardware behavior still needs validation on each OS.

## Verification

The standalone tests inject SFML-style snapshots to check remapping, analog
ranges, POV diagonals, combined/separate triggers, disconnect release, and focus
suppression. They also validate JSON and smoke-test SFML's native joystick API.
They need no physical gamepad and no Qt:

```sh
cmake -S SerialPrograms/Tests/Gamepad -B build/sfml-tests
cmake --build build/sfml-tests --config Release
ctest --test-dir build/sfml-tests -C Release --output-on-failure
```

For hardware verification on each OS, check partial/full stick motion, all
buttons and triggers, D-pad diagonals, connecting before/after launch, unplugging
while holding controls, focus loss, console switching and automation start/stop.
Use raw input logging to confirm the mapping before sending commands to a console.
