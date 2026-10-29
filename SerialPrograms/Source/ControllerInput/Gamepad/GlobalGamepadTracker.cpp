#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "GamepadInput_SFML.h"
#include "GlobalGamepadTracker.h"

namespace PokemonAutomation{

GamepadTracker& global_gamepad_tracker(){
    static GamepadTracker tracker;
    return tracker;
}

void GamepadTracker::start(){
    if (m_initialized) return;
    try{
        m_mapping = load_gamepad_mapping(SETTINGS_PATH() + "SFML-Gamepad.json");
    }catch (const std::exception& error){
        global_logger_tagged().log(std::string("SFML gamepad disabled: ") + error.what());
        return;
    }
    m_initialized = true;
    m_stopping.store(false, std::memory_order_release);
    m_thread = GlobalThreadPools::unlimited_realtime().dispatch_now_blocking([this]{ thread_loop(); });
}

void GamepadTracker::stop() noexcept{
    m_stopping.store(true, std::memory_order_release);
    { std::lock_guard<Mutex> guard(m_lock); }
    m_cv.notify_all();
    if (m_thread) m_thread.wait_and_ignore_exceptions();
    m_initialized = false;
}

void GamepadTracker::thread_loop(){
    unsigned int pad = sf::Joystick::Count;
    GamepadInputState last;
    GamepadInputGate gate;
    auto refresh = std::chrono::steady_clock::now();
    auto next_raw_log = refresh;
    while (!m_stopping.load(std::memory_order_acquire)){
        // No SFML window is created: explicitly refresh joystick state.
        sf::Joystick::update();
        auto now = std::chrono::steady_clock::now();
        if (pad < sf::Joystick::Count && !sf::Joystick::isConnected(pad)){
            global_logger_tagged().log("SFML gamepad disconnected.");
            pad = sf::Joystick::Count;
            gate.reset();
        }
        if (pad == sf::Joystick::Count){
            for (unsigned int c = 0; c < sf::Joystick::Count; c++){
                if ((m_mapping.joystick < 0 || c == (unsigned int)m_mapping.joystick) && sf::Joystick::isConnected(c)){
                    pad = c;
                    gate.reset();
                    auto id = sf::Joystick::getIdentification(c);
                    global_logger_tagged().log("SFML gamepad slot " + std::to_string(c) + ": "
                        + id.name.toAnsiString() + " (vendor=" + std::to_string(id.vendorId)
                        + ", product=" + std::to_string(id.productId) + ", buttons="
                        + std::to_string(sf::Joystick::getButtonCount(c)) + ")");
                    break;
                }
            }
        }
        RawGamepadState raw = read_sfml_joystick(pad);
        GamepadInputState current = map_gamepad(raw, m_mapping);
        if (m_mapping.log_raw_input && raw.connected && now >= next_raw_log){
            next_raw_log = now + std::chrono::milliseconds(500);
            const char* names[] = {"X", "Y", "Z", "R", "U", "V", "PovX", "PovY"};
            std::ostringstream line;
            line << "SFML raw input: buttons=";
            for (size_t c = 0; c < raw.buttons.size(); c++) if (raw.buttons[c]) line << c << ',';
            for (size_t c = 0; c < raw.axes.size(); c++) if (raw.has_axis[c]) line << ' ' << names[c] << '=' << raw.axes[c];
            global_logger_tagged().log(line.str());
        }
        if (m_reset.exchange(false, std::memory_order_acq_rel)) gate.reset();
        // Require release after focus/console changes or automation. A held
        // button must never leak to the newly selected console.
        gate.filter(current, !m_listeners.empty());
        if (current != last || (!current.is_neutral() && now >= refresh)){
            try{
                m_listeners.run_method(&ControllerInputListener::run_controller_input, current);
            }catch (Exception& error){
                error.log(global_logger_tagged());
            }
            last = current;
            refresh = now + std::chrono::seconds(1);
        }
        std::unique_lock<Mutex> lock(m_lock);
        if (!m_stopping.load(std::memory_order_acquire)) m_cv.wait_for(lock, std::chrono::milliseconds(8));
    }
}

}
