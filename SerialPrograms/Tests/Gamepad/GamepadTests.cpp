#include <cstdlib>
#include <iostream>
#include "ControllerInput/Gamepad/GamepadInput_SDL.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Gamepad.h"

using namespace PokemonAutomation;

static void check(bool passed, const char* message){
    if (!passed){ std::cerr << message << ": " << SDL_GetError() << '\n'; std::exit(1); }
}

int main(){
    double x, y;
    gamepad_stick(1000, -1000, x, y);
    check(x == 0 && y == 0, "Stick drift must be neutral");
    gamepad_stick(-32768, 0, x, y);
    check(x == -1 && y == 0, "Negative full-scale stick");
    gamepad_stick(0, -32768, x, y);
    check(x == 0 && y == 1, "Up must use positive output Y");
    gamepad_stick(16384, 0, x, y);
    check(x > 0 && x < 1 && y == 0, "Preserve analog magnitude");
    gamepad_stick(32767, 32767, x, y);
    check(std::abs(std::hypot(x, y) - 1) < 1e-6, "Clamp diagonal magnitude");

    check(SDL_Init(SDL_INIT_GAMEPAD), "Initialize input");
    SDL_SetJoystickEventsEnabled(false);
    SDL_SetGamepadEventsEnabled(false);
    SDL_VirtualJoystickDesc desc{};
    desc.version = sizeof(desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
    desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    desc.axis_mask = (1u << SDL_GAMEPAD_AXIS_COUNT) - 1;
    desc.button_mask = (1u << SDL_GAMEPAD_BUTTON_COUNT) - 1;
    desc.name = "Pokemon Automation test gamepad";
    SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
    check(id != 0 && SDL_IsGamepad(id), "Recognize hot-plugged gamepad");
    SDL_Gamepad* pad = SDL_OpenGamepad(id);
    check(pad != nullptr, "Open virtual gamepad");
    SDL_Joystick* joystick = SDL_GetGamepadJoystick(pad);
    // Virtual raw trigger axes use the full signed range, mapped to 0..32767.
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, -32768);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, -32768);
    SDL_UpdateGamepads();
    check(read_gamepad(pad).is_neutral(), "Centered virtual pad is neutral");
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true);
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_DPAD_UP, true);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 16384);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTY, -32768);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 32767);
    SDL_UpdateGamepads();
    auto state = read_gamepad(pad);
    check(state.pressed(GamepadButton::South) && state.pressed(GamepadButton::Up), "Buttons and D-pad");
    check(state.left_x > 0 && state.left_y > 0, "Analog axes and Y inversion");
    check(state.right_trigger && !state.left_trigger, "Independent trigger mapping");
    auto deltas = NintendoSwitch::gamepad_to_deltas(state);
    check(deltas.buttons == (NintendoSwitch::BUTTON_B | NintendoSwitch::BUTTON_ZR), "Nintendo physical positions and trigger output");
    check(deltas.dpad_x == 0 && deltas.dpad_y == -1, "Nintendo D-pad up");
    GamepadInputState diagonal = state;
    diagonal.buttons[(size_t)GamepadButton::Right] = true;
    deltas = NintendoSwitch::gamepad_to_deltas(diagonal);
    check(deltas.dpad_x == 1 && deltas.dpad_y == -1, "Nintendo D-pad diagonal");
    GamepadInputState copy;
    copy = state;
    check(copy == state && !copy.is_neutral(), "Copy complete snapshot");
    copy.clear();
    check(copy.is_neutral() && copy != state, "Release complete snapshot");
    GamepadInputGate gate;
    copy = state;
    gate.filter(copy, true);
    check(copy.is_neutral(), "Held input must be suppressed on initial focus");
    gate.filter(copy, true);
    copy = state;
    gate.filter(copy, true);
    check(copy == state, "Released pad resumes input");
    gate.reset();
    gate.filter(copy, true);
    check(copy.is_neutral(), "Console switch must release held input");
    gate.filter(copy, true);
    copy = state;
    gate.filter(copy, false);
    check(copy.is_neutral(), "No listeners must disarm input");
    copy = state;
    gate.filter(copy, true);
    check(copy.is_neutral(), "Returning focus still requires release");
    check(SDL_DetachVirtualJoystick(id), "Unplug virtual gamepad");
    SDL_UpdateGamepads();
    check(!SDL_GamepadConnected(pad), "Detect disconnect");
    SDL_CloseGamepad(pad);
    SDL_Quit();
    std::cout << "Gamepad tests passed\n";
}
