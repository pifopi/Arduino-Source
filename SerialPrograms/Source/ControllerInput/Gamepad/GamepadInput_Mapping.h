#ifndef PokemonAutomation_GamepadInput_Mapping_H
#define PokemonAutomation_GamepadInput_Mapping_H

#include <string>
#include "GamepadInput_State.h"

namespace PokemonAutomation{

// Indices follow SFML's Axis enum: X, Y, Z, R, U, V, PovX, PovY.
struct RawGamepadState{
    bool connected = false;
    std::array<bool, 32> buttons{};
    std::array<bool, 8> has_axis{};
    std::array<float, 8> axes{};
};

struct GamepadAxisBinding{
    int axis = -1;
    bool invert = false;
    float read(const RawGamepadState& raw) const{
        if (axis < 0 || axis >= 8 || !raw.has_axis[axis]) return 0;
        float value = raw.axes[axis];
        if (!std::isfinite(value)) return 0;
        return std::clamp(value, -100.f, 100.f) * (invert ? -1.f : 1.f);
    }
};

struct GamepadTriggerBinding{
    GamepadAxisBinding axis;
    int button = -1;
    float rest = 0;
    float threshold = 50;
    bool pressed(const RawGamepadState& raw) const{
        if (button >= 0 && button < 32 && raw.buttons[button]) return true;
        return axis.axis >= 0 && axis.axis < 8 && raw.has_axis[axis.axis]
            && std::isfinite(raw.axes[axis.axis])
            && (axis.read(raw) - rest) > threshold;
    }
};

struct GamepadMapping{
    GamepadMapping();
    // -1 selects the first connected joystick; otherwise select slot 0..7.
    int joystick = -1;
    bool log_raw_input = false;
    std::array<int, (size_t)GamepadButton::Count> buttons;
    GamepadAxisBinding left_x{0}, left_y{1}, right_x{4}, right_y{3};
    GamepadAxisBinding pov_x{6}, pov_y{7};
    GamepadTriggerBinding left_trigger{{2, false}};
    GamepadTriggerBinding right_trigger{{2, true}};
};

GamepadMapping parse_gamepad_mapping(const std::string& json);
std::string default_gamepad_mapping_json();
// Writes defaults on first use, validates existing files on subsequent starts.
GamepadMapping load_gamepad_mapping(const std::string& path);
GamepadInputState map_gamepad(const RawGamepadState& raw, const GamepadMapping& mapping);

}
#endif
