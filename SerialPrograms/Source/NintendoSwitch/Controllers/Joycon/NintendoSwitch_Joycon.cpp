/*  Nintendo Switch Joycon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Strings/StringTools.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "ControllerInput/ControllerInput.h"
#include "ControllerInput/Keyboard/KeyboardInput_State.h"
#include "Controllers/ControllerTypes.h"
#include "Controllers/RumbleListener.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Gamepad.h"
#include "NintendoSwitch_JoyconState.h"
#include "NintendoSwitch_Joycon.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;


const char JoyconController::NAME[] = "Nintendo Switch: Joycon";
const char LeftJoycon::NAME[] = "Nintendo Switch: Left Joycon";
const char RightJoycon::NAME[] = "Nintendo Switch: Right Joycon";



struct JoyconController::Data{
    ListenerSet<RumbleListener> m_rumble_listeners;
    std::map<KeyboardKey, JoyconDeltas> m_keyboard_mapping;
    bool m_left = false;
};



void JoyconController::add_listener(RumbleListener& listener){
    m_data->m_rumble_listeners.add(listener);
}
void JoyconController::remove_listener(RumbleListener& listener){
    m_data->m_rumble_listeners.remove(listener);
}




JoyconController::JoyconController(Logger& logger, ControllerClass controller_class)
    : m_data(CONSTRUCT_TOKEN)
{
    m_data->m_left = controller_class == ControllerClass::NintendoSwitch_LeftJoycon;
    std::vector<std::shared_ptr<EditableTableRow>> mapping =
        controller_class == ControllerClass::NintendoSwitch_LeftJoycon
            ? ConsoleSettings::instance().KEYBOARD_MAPPINGS.LEFT_JOYCON2.current_refs()
            : ConsoleSettings::instance().KEYBOARD_MAPPINGS.RIGHT_JOYCON2.current_refs();

    for (const auto& deltas : mapping){
        const JoyconFromKeyboardTableRow& row = static_cast<const JoyconFromKeyboardTableRow&>(*deltas);
        m_data->m_keyboard_mapping[row.key] += row.snapshot();
    }
}
JoyconController::~JoyconController(){
}





bool JoyconController::run_string_command(Milliseconds duration, const std::string& command){
    //  stop
    //  replace
    //  wait
    //  A,B
    //  A,B|JS:+0.5:-0.5

    std::vector<std::string> tokens = StringTools::split(command, "|");
    if (tokens.empty()){
        return false;
    }

    if (tokens[0] == "stop"){
        cancel_all_commands();
        return true;
    }
    if (tokens[0] == "replace"){
        replace_on_next_command();
        return true;
    }
    if (tokens[0] == "wait"){
        issue_nop(nullptr, duration);
        return true;
    }

    JoyconState state;

    for (const std::string& token : tokens){
        if (token.starts_with("JS")){
            std::vector<std::string> args = StringTools::split(token, ":");
            if (args.size() != 3){
                return false;
            }
            state.joystick.x = std::atof(args[1].data());
            state.joystick.y = std::atof(args[2].data());
            continue;
        }
        state.buttons |= string_to_button(token, ",");
    }

    state.execute(nullptr, true, *this, duration);

    return true;
}


void JoyconController::run_controller_input(const ControllerInputState& state){

    if (state.type() != ControllerInputType::HID_Keyboard && state.type() != ControllerInputType::StandardGamepad){
        return;
    }

    JoyconDeltas deltas;

    JoyconState controller_state;
    if (state.type() == ControllerInputType::StandardGamepad){
        const auto& input = static_cast<const GamepadInputState&>(state);
        ProControllerState pro;
        gamepad_to_pro_controller(input, pro);
        controller_state.buttons = pro.buttons;
        if (m_data->m_left){
            // The left Joy-Con's directional buttons occupy its face cluster.
            if (input.pressed(GamepadButton::Up)) controller_state.buttons |= BUTTON_UP;
            if (input.pressed(GamepadButton::Down)) controller_state.buttons |= BUTTON_DOWN;
            if (input.pressed(GamepadButton::Left)) controller_state.buttons |= BUTTON_LEFT;
            if (input.pressed(GamepadButton::Right)) controller_state.buttons |= BUTTON_RIGHT;
            controller_state.joystick = pro.left_joystick;
        }else{
            controller_state.joystick = pro.right_joystick;
        }
    }else{
        const KeyboardInputState& lstate = static_cast<const KeyboardInputState&>(state);
        const std::map<KeyboardKey, JoyconDeltas>& map = m_data->m_keyboard_mapping;

    //    cout << "keys() = " << lstate.keys().size() << endl;

        for (KeyboardKey key : lstate.keys()){
            auto iter = map.find(key);
            if (iter != map.end()){
                deltas += iter->second;
            }
        }

        deltas.to_state(controller_state);
    }

    WallClock timestamp;
    if (controller_state.is_neutral()){
        timestamp = current_time();
        cancel_all_commands();
    }else{
        replace_on_next_command();

        timestamp = current_time();
        controller_state.execute(nullptr, false, *this, 2000ms);
    }

    on_command_input(timestamp, controller_state);
}


void JoyconController::on_rumble(double magnitude){
    m_data->m_rumble_listeners.run_method(&RumbleListener::on_rumble, magnitude);
}






ControllerClass LeftJoycon::controller_class() const noexcept{
    return ControllerClass::NintendoSwitch_LeftJoycon;
}
ControllerClass RightJoycon::controller_class() const noexcept{
    return ControllerClass::NintendoSwitch_RightJoycon;
}





}
}
