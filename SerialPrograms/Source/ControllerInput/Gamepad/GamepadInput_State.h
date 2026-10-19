/*  Standard gamepad input (physical button positions, independent of labels). */
#ifndef PokemonAutomation_GamepadInput_State_H
#define PokemonAutomation_GamepadInput_State_H

#include <algorithm>
#include <array>
#include <cmath>
#include "ControllerInput/ControllerInput.h"

namespace PokemonAutomation{

enum class GamepadButton{
    South, East, West, North, Back, Guide, Start, LeftStick, RightStick,
    LeftShoulder, RightShoulder, Up, Down, Left, Right, Capture, Count
};

class GamepadInputState final : public ControllerInputState{
public:
    GamepadInputState() : ControllerInputState(ControllerInputType::StandardGamepad) {}
    std::array<bool, (size_t)GamepadButton::Count> buttons{};
    double left_x = 0, left_y = 0, right_x = 0, right_y = 0;
    bool left_trigger = false, right_trigger = false;

    GamepadInputState& operator=(const GamepadInputState& other){
        buttons = other.buttons;
        left_x = other.left_x; left_y = other.left_y;
        right_x = other.right_x; right_y = other.right_y;
        left_trigger = other.left_trigger; right_trigger = other.right_trigger;
        return *this;
    }

    bool pressed(GamepadButton button) const{ return buttons[(size_t)button]; }
    void clear() override{
        buttons.fill(false);
        left_x = left_y = right_x = right_y = 0;
        left_trigger = right_trigger = false;
    }
    bool is_neutral() const override{
        for (bool button : buttons){ if (button) return false; }
        return left_x == 0 && left_y == 0 && right_x == 0 && right_y == 0
            && !left_trigger && !right_trigger;
    }
    bool operator==(const ControllerInputState& state) const override{
        const auto* other = dynamic_cast<const GamepadInputState*>(&state);
        return other && buttons == other->buttons
            && left_x == other->left_x && left_y == other->left_y
            && right_x == other->right_x && right_y == other->right_y
            && left_trigger == other->left_trigger && right_trigger == other->right_trigger;
    }
};

// Rescale outside a radial deadzone so centered sticks are exactly neutral,
// while retaining the entire analog range. SDL's positive Y points down.
inline void gamepad_stick(int x, int y, double& out_x, double& out_y){
    double nx = x / (x < 0 ? 32768.0 : 32767.0);
    double ny = y / (y < 0 ? 32768.0 : 32767.0);
    double magnitude = std::sqrt(nx * nx + ny * ny);
    constexpr double deadzone = 0.15;
    if (magnitude <= deadzone){ out_x = out_y = 0; return; }
    double scale = (std::min(magnitude, 1.0) - deadzone) / ((1.0 - deadzone) * magnitude);
    out_x = nx * scale;
    out_y = -ny * scale;
}

class GamepadInputGate{
public:
    void reset(){ m_armed = false; }
    void filter(GamepadInputState& state, bool has_listeners){
        if (!has_listeners) m_armed = false;
        if (!m_armed){
            m_armed = has_listeners && state.is_neutral();
            state.clear();
        }
    }
private:
    bool m_armed = false;
};

}
#endif
