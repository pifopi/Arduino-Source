#ifndef PokemonAutomation_GamepadInput_SFML_H
#define PokemonAutomation_GamepadInput_SFML_H
#include <SFML/Window/Joystick.hpp>
#include "GamepadInput_Mapping.h"

namespace PokemonAutomation{

inline RawGamepadState read_sfml_joystick(unsigned int joystick){
    RawGamepadState raw;
    if (joystick >= sf::Joystick::Count || !sf::Joystick::isConnected(joystick)) return raw;
    raw.connected = true;
    for (unsigned int c = 0; c < std::min(sf::Joystick::getButtonCount(joystick), sf::Joystick::ButtonCount); c++){
        raw.buttons[c] = sf::Joystick::isButtonPressed(joystick, c);
    }
    static_assert(sf::Joystick::AxisCount == 8 && sf::Joystick::ButtonCount == 32);
    for (unsigned int c = 0; c < sf::Joystick::AxisCount; c++){
        auto axis = static_cast<sf::Joystick::Axis>(c);
        raw.has_axis[c] = sf::Joystick::hasAxis(joystick, axis);
        if (raw.has_axis[c]) raw.axes[c] = sf::Joystick::getAxisPosition(joystick, axis);
    }
    return raw;
}

}
#endif
