#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "GamepadInput_Mapping.h"

namespace PokemonAutomation{
namespace{
using Json = nlohmann::json;
constexpr const char* BUTTON_NAMES[] = {
    "South", "East", "West", "North", "Back", "Guide", "Start", "LeftStick", "RightStick",
    "LeftShoulder", "RightShoulder", "Up", "Down", "Left", "Right", "Capture"
};
constexpr const char* AXIS_NAMES[] = {"X", "Y", "Z", "R", "U", "V", "PovX", "PovY"};
static_assert(std::size(BUTTON_NAMES) == (size_t)GamepadButton::Count);

int integer(const Json& value, int low, int high){
    if (!value.is_number_integer() || value < low || value > high){
        throw std::runtime_error("Gamepad button/joystick index is out of range.");
    }
    return value.get<int>();
}
GamepadAxisBinding axis(const Json& value){
    GamepadAxisBinding result;
    std::string name = value.at("axis").get<std::string>();
    if (name != "None"){
        for (int c = 0; c < 8; c++) if (name == AXIS_NAMES[c]) result.axis = c;
        if (result.axis < 0) throw std::runtime_error("Unknown gamepad axis: " + name);
    }
    result.invert = value.value("invert", false);
    return result;
}
GamepadTriggerBinding trigger(const Json& value){
    GamepadTriggerBinding result;
    result.axis = axis(value);
    result.button = integer(value.value("button", Json(-1)), -1, 31);
    result.rest = value.value("rest", 0.f);
    result.threshold = value.value("threshold", 50.f);
    if (!std::isfinite(result.rest) || std::abs(result.rest) > 100
        || !std::isfinite(result.threshold) || result.threshold <= 0 || result.threshold > 200){
        throw std::runtime_error("Invalid gamepad trigger range.");
    }
    return result;
}
Json axis_json(GamepadAxisBinding binding){
    return {{"axis", binding.axis < 0 ? "None" : AXIS_NAMES[binding.axis]}, {"invert", binding.invert}};
}
Json trigger_json(GamepadTriggerBinding binding){
    Json result = axis_json(binding.axis);
    result["button"] = binding.button;
    result["rest"] = binding.rest;
    result["threshold"] = binding.threshold;
    return result;
}
}

GamepadMapping::GamepadMapping(){
    buttons.fill(-1);
    // Common Windows Xbox/DirectInput layout. Guide/Capture are unavailable.
    const int defaults[] = {0, 1, 2, 3, 6, -1, 7, 8, 9, 4, 5, -1, -1, -1, -1, -1};
    std::copy(std::begin(defaults), std::end(defaults), buttons.begin());
#if defined(__linux__)
    // Common Linux xpad layout: independent Z/R triggers, U/V right stick.
    right_y.axis = 5;
    pov_y.invert = true;
    left_trigger = {{2, false}, -1, -100, 100};
    right_trigger = {{3, false}, -1, -100, 100};
    buttons[(size_t)GamepadButton::Guide] = 8;
    buttons[(size_t)GamepadButton::LeftStick] = 9;
    buttons[(size_t)GamepadButton::RightStick] = 10;
#endif
}

GamepadMapping parse_gamepad_mapping(const std::string& text){
    Json json = Json::parse(text);
    if (!json.is_object()) throw std::runtime_error("Gamepad mapping must be a JSON object.");
    const char* keys[] = {"joystick", "log_raw_input", "buttons", "left_x", "left_y", "right_x", "right_y", "pov_x", "pov_y", "left_trigger", "right_trigger"};
    for (auto entry = json.begin(); entry != json.end(); ++entry){
        if (std::find(std::begin(keys), std::end(keys), entry.key()) == std::end(keys)){
            throw std::runtime_error("Unknown gamepad mapping key: " + entry.key());
        }
    }
    GamepadMapping mapping;
    mapping.joystick = integer(json.value("joystick", Json(-1)), -1, 7);
    mapping.log_raw_input = json.value("log_raw_input", false);
    if (json.contains("buttons")){
        const Json& buttons = json.at("buttons");
        if (!buttons.is_object()) throw std::runtime_error("Gamepad buttons must be an object.");
        for (auto entry = buttons.begin(); entry != buttons.end(); ++entry){
            auto name = std::find(std::begin(BUTTON_NAMES), std::end(BUTTON_NAMES), entry.key());
            if (name == std::end(BUTTON_NAMES)) throw std::runtime_error("Unknown gamepad button: " + entry.key());
            mapping.buttons[name - std::begin(BUTTON_NAMES)] = integer(entry.value(), -1, 31);
        }
    }
    if (json.contains("left_x")) mapping.left_x = axis(json.at("left_x"));
    if (json.contains("left_y")) mapping.left_y = axis(json.at("left_y"));
    if (json.contains("right_x")) mapping.right_x = axis(json.at("right_x"));
    if (json.contains("right_y")) mapping.right_y = axis(json.at("right_y"));
    if (json.contains("pov_x")) mapping.pov_x = axis(json.at("pov_x"));
    if (json.contains("pov_y")) mapping.pov_y = axis(json.at("pov_y"));
    if (json.contains("left_trigger")) mapping.left_trigger = trigger(json.at("left_trigger"));
    if (json.contains("right_trigger")) mapping.right_trigger = trigger(json.at("right_trigger"));
    return mapping;
}

std::string default_gamepad_mapping_json(){
    GamepadMapping mapping;
    Json json = {{"joystick", mapping.joystick}, {"log_raw_input", false}};
    for (size_t c = 0; c < mapping.buttons.size(); c++) json["buttons"][BUTTON_NAMES[c]] = mapping.buttons[c];
    json["left_x"] = axis_json(mapping.left_x); json["left_y"] = axis_json(mapping.left_y);
    json["right_x"] = axis_json(mapping.right_x); json["right_y"] = axis_json(mapping.right_y);
    json["pov_x"] = axis_json(mapping.pov_x); json["pov_y"] = axis_json(mapping.pov_y);
    json["left_trigger"] = trigger_json(mapping.left_trigger);
    json["right_trigger"] = trigger_json(mapping.right_trigger);
    return json.dump(4) + "\n";
}

GamepadMapping load_gamepad_mapping(const std::string& path){
    auto native_path = std::filesystem::u8path(path);
    if (!std::filesystem::exists(native_path)){
        std::ofstream file(native_path);
        if (!file || !(file << default_gamepad_mapping_json())){
            throw std::runtime_error("Cannot write gamepad mapping: " + path);
        }
        return GamepadMapping();
    }
    std::ifstream file(native_path);
    if (!file) throw std::runtime_error("Cannot read gamepad mapping: " + path);
    return parse_gamepad_mapping(std::string(std::istreambuf_iterator<char>(file), {}));
}

GamepadInputState map_gamepad(const RawGamepadState& raw, const GamepadMapping& mapping){
    GamepadInputState result;
    if (!raw.connected) return result;
    for (size_t c = 0; c < result.buttons.size(); c++){
        int index = mapping.buttons[c];
        result.buttons[c] = index >= 0 && index < 32 && raw.buttons[index];
    }
    result.buttons[(size_t)GamepadButton::Right] |= mapping.pov_x.read(raw) > 50;
    result.buttons[(size_t)GamepadButton::Left] |= mapping.pov_x.read(raw) < -50;
    result.buttons[(size_t)GamepadButton::Up] |= mapping.pov_y.read(raw) > 50;
    result.buttons[(size_t)GamepadButton::Down] |= mapping.pov_y.read(raw) < -50;
    gamepad_stick(mapping.left_x.read(raw) / 100., mapping.left_y.read(raw) / 100., result.left_x, result.left_y);
    gamepad_stick(mapping.right_x.read(raw) / 100., mapping.right_y.read(raw) / 100., result.right_x, result.right_y);
    result.left_trigger = mapping.left_trigger.pressed(raw);
    result.right_trigger = mapping.right_trigger.pressed(raw);
    return result;
}

}
