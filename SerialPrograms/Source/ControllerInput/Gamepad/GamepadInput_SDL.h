#ifndef PokemonAutomation_GamepadInput_SDL_H
#define PokemonAutomation_GamepadInput_SDL_H
#include <SDL3/SDL.h>
#include "GamepadInput_State.h"
namespace PokemonAutomation{
inline GamepadInputState read_gamepad(SDL_Gamepad* pad){
    GamepadInputState state;
    const SDL_GamepadButton mapping[] = {
        SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST,
        SDL_GAMEPAD_BUTTON_WEST, SDL_GAMEPAD_BUTTON_NORTH,
        SDL_GAMEPAD_BUTTON_BACK, SDL_GAMEPAD_BUTTON_GUIDE, SDL_GAMEPAD_BUTTON_START,
        SDL_GAMEPAD_BUTTON_LEFT_STICK, SDL_GAMEPAD_BUTTON_RIGHT_STICK,
        SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
        SDL_GAMEPAD_BUTTON_DPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_DOWN,
        SDL_GAMEPAD_BUTTON_DPAD_LEFT, SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_MISC1
    };
    static_assert(std::size(mapping) == (size_t)GamepadButton::Count);
    for (size_t c = 0; c < state.buttons.size(); c++){
        state.buttons[c] = SDL_GetGamepadButton(pad, mapping[c]);
    }
    gamepad_stick(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX),
        SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY), state.left_x, state.left_y);
    gamepad_stick(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX),
        SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY), state.right_x, state.right_y);
    state.left_trigger = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 16384;
    state.right_trigger = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 16384;
    return state;
}

}
#endif

