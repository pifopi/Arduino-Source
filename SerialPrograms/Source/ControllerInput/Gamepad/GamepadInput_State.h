/*  Gamepad Input State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Gamepad_GamepadInput_State_H
#define PokemonAutomation_ControllerInput_Gamepad_GamepadInput_State_H

#include "ControllerInput/ControllerInput.h"
#include "Common/ControllerStates/NintendoSwitch_WiredController_State.h"

namespace PokemonAutomation{



class GamepadInputState final : public ControllerInputState{
public:
    GamepadInputState(GamepadInputState&& x) noexcept
        : ControllerInputState(ControllerInputType::StandardGamepad)
        , m_state(x.m_state)
    {}
    void operator=(GamepadInputState&& x) noexcept{
        if (this == &x){
            return;
        }
        m_state = x.m_state;
    }


public:
    GamepadInputState()
        : ControllerInputState(ControllerInputType::StandardGamepad)
    {}


    virtual void clear() override{
        m_state = pabb_NintendoSwitch_WiredController_State_NEUTRAL_STATE;
    }
    virtual bool is_neutral() const override{
        return pabb_NintendoSwitch_WiredController_State_equals(&m_state, &pabb_NintendoSwitch_WiredController_State_NEUTRAL_STATE);
    }
    virtual bool operator==(const ControllerInputState& state) const override{
        const GamepadInputState* x = dynamic_cast<const GamepadInputState*>(&state);
        if (x == nullptr){
            return false;
        }
        return pabb_NintendoSwitch_WiredController_State_equals(&m_state, &x->m_state);
    }


private :
    pabb_NintendoSwitch_WiredController_State m_state{ pabb_NintendoSwitch_WiredController_State_NEUTRAL_STATE };
};




}
#endif
