#include <cstdlib>
#include <iostream>
#include <limits>
#include <filesystem>
#include <fstream>
#include <chrono>
#include "ControllerInput/Gamepad/GamepadInput_SFML.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Gamepad.h"

using namespace PokemonAutomation;

static void check(bool passed, const char* message){
    if (!passed){ std::cerr << message << '\n'; std::exit(1); }
}

int main(){
    double x, y;
    gamepad_stick(0.03, -0.03, x, y);
    check(x == 0 && y == 0, "Stick drift must be neutral");
    gamepad_stick(-1, 0, x, y);
    check(x == -1 && y == 0, "Negative full-scale stick");
    gamepad_stick(0, -1, x, y);
    check(x == 0 && y == 1, "Up must use positive output Y");
    gamepad_stick(0.5, 0, x, y);
    check(x > 0 && x < 1 && y == 0, "Preserve analog magnitude");
    gamepad_stick(1, 1, x, y);
    check(std::abs(std::hypot(x, y) - 1) < 1e-6, "Clamp diagonal magnitude");

    // Inject raw SFML-style snapshots; no OS virtual-controller driver needed.
    GamepadMapping mapping;
    mapping.left_trigger = {{2, false}, -1, 0, 50};
    mapping.right_trigger = {{2, true}, -1, 0, 50};
    mapping.pov_y.invert = false;
    RawGamepadState raw;
    raw.connected = true;
    raw.has_axis.fill(true);
    check(map_gamepad(raw, mapping).is_neutral(), "Centered raw pad is neutral");
    raw.buttons[0] = true;
    raw.axes[7] = 100;
    raw.axes[0] = 50;
    raw.axes[1] = -100;
    raw.axes[2] = -100;
    auto state = map_gamepad(raw, mapping);
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
    raw.connected = false;
    check(map_gamepad(raw, mapping).is_neutral(), "Disconnect releases all controls");
    raw.connected = true;
    raw.axes[2] = 100;
    auto left_trigger = map_gamepad(raw, mapping);
    check(left_trigger.left_trigger && !left_trigger.right_trigger, "Combined axis left trigger");
    mapping.left_trigger = {{2, false}, -1, -100, 100};
    mapping.right_trigger = {{3, false}, -1, -100, 100};
    raw.axes[2] = raw.axes[3] = -100;
    check(!map_gamepad(raw, mapping).left_trigger && !map_gamepad(raw, mapping).right_trigger, "Split trigger resting positions");
    raw.axes[2] = raw.axes[3] = 100;
    check(map_gamepad(raw, mapping).left_trigger && map_gamepad(raw, mapping).right_trigger, "Independent simultaneous triggers");
    raw.has_axis[2] = false;
    check(!map_gamepad(raw, mapping).left_trigger, "Missing trigger axis stays released");
    raw.has_axis[2] = true;
    raw.axes[2] = std::numeric_limits<float>::quiet_NaN();
    check(!map_gamepad(raw, mapping).left_trigger, "Non-finite trigger stays released");
    auto remapped = parse_gamepad_mapping(R"({"joystick":3,"buttons":{"South":5},"left_x":{"axis":"V","invert":true},"left_trigger":{"axis":"None","button":12}})");
    raw.buttons.fill(false);
    raw.buttons[5] = raw.buttons[12] = true;
    raw.axes[5] = 100;
    auto remap_state = map_gamepad(raw, remapped);
    check(remapped.joystick == 3 && remap_state.pressed(GamepadButton::South), "Remap joystick and buttons");
    check(remap_state.left_x < 0 && remap_state.left_trigger, "Remap and invert axes; button triggers");
    auto defaults = parse_gamepad_mapping(default_gamepad_mapping_json());
    check(defaults.buttons == GamepadMapping().buttons && defaults.right_y.axis == GamepadMapping().right_y.axis, "Default mapping round trip");
    const char* invalid[] = {
        R"({"buttons":{"South":32}})", R"({"joystick":8})",
        R"({"left_x":{"axis":"Bogus"}})", R"({"buttons":{"South":0.5}})",
        R"({"left_trigger":{"axis":"Z","threshold":0}})", R"({"buttons":{"Typo":1}})",
        R"([])", R"({"left_trigger":{"axis":"Z","rest":-101}})", R"({"typo":true})"
    };
    for (const char* json : invalid){
        bool rejected = false;
        try{ parse_gamepad_mapping(json); }catch (const std::exception&){ rejected = true; }
        check(rejected, "Reject invalid mappings");
    }
    auto path = std::filesystem::temp_directory_path() /
        ("pa-sfml-gamepad-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".json");
    auto utf8_path = path.u8string();
    std::string path_text(utf8_path.begin(), utf8_path.end());
    auto loaded = load_gamepad_mapping(path_text);
    check(std::filesystem::exists(path) && loaded.buttons == defaults.buttons, "Create default mapping on first use");
    { std::ofstream file(path); file << R"({"joystick":5})"; }
    check(load_gamepad_mapping(path_text).joystick == 5, "Load edited mapping");
    { std::ofstream file(path); file << "broken json"; }
    bool rejected = false;
    try{ load_gamepad_mapping(path_text); }catch (const std::exception&){ rejected = true; }
    std::filesystem::remove(path);
    check(rejected, "Report corrupt mapping without silently replacing it");
    // Smoke-test the actual SFML backend, even with no controller attached.
    sf::Joystick::update();
    check(!read_sfml_joystick(sf::Joystick::Count).connected, "Invalid SFML slot stays neutral");
    for (unsigned int slot = 0; slot < sf::Joystick::Count; slot++){
        auto snapshot = read_sfml_joystick(slot);
        check(snapshot.connected == sf::Joystick::isConnected(slot), "Read SFML connection state");
    }
    std::cout << "SFML gamepad tests passed\n";
}
