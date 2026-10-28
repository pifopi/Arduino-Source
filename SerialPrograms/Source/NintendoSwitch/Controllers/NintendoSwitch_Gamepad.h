#ifndef PokemonAutomation_NintendoSwitch_Gamepad_H
#define PokemonAutomation_NintendoSwitch_Gamepad_H

#include "ControllerInput/Gamepad/GamepadInput_State.h"
#include "NintendoSwitch_VirtualControllerState.h"
#include "Procon/NintendoSwitch_ProControllerState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

inline ProControllerDeltas gamepad_to_deltas(const GamepadInputState& input){
    // Match physical Nintendo positions (south=B, east=A), regardless of
    // Xbox/PlayStation/Nintendo labels on the input device.
    static constexpr Button mapping[] = {
        BUTTON_B, BUTTON_A, BUTTON_Y, BUTTON_X, BUTTON_MINUS, BUTTON_HOME,
        BUTTON_PLUS, BUTTON_LCLICK, BUTTON_RCLICK, BUTTON_L, BUTTON_R,
        BUTTON_NONE, BUTTON_NONE, BUTTON_NONE, BUTTON_NONE, BUTTON_CAPTURE
    };
    static_assert(std::size(mapping) == (size_t)GamepadButton::Count);
    ProControllerDeltas deltas;
    for (size_t c = 0; c < input.buttons.size(); c++){
        if (input.buttons[c]) deltas.buttons |= mapping[c];
    }
    if (input.left_trigger) deltas.buttons |= BUTTON_ZL;
    if (input.right_trigger) deltas.buttons |= BUTTON_ZR;
    deltas.dpad_x = (int)input.pressed(GamepadButton::Right) - (int)input.pressed(GamepadButton::Left);
    deltas.dpad_y = (int)input.pressed(GamepadButton::Down) - (int)input.pressed(GamepadButton::Up);
    return deltas;
}

inline void gamepad_to_pro_controller(const GamepadInputState& input, ProControllerState& output){
    ProControllerDeltas deltas = gamepad_to_deltas(input);
    deltas.to_state(output);
    output.left_joystick = {input.left_x, input.left_y};
    output.right_joystick = {input.right_x, input.right_y};
}

}
}
#endif
