/*  Gamepad Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "GamepadInput_State.h"
#include "GamepadTracker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




GamepadTracker& global_gamepad_tracker(){
    static GamepadTracker tracker;
    return tracker;
}




GamepadTracker::~GamepadTracker(){
    stop();
}
GamepadTracker::GamepadTracker()
    : m_logger(global_logger_raw(), "Gamepad")
    , m_stopping(false)
    , m_thread([this]{
        run_with_catch(
            "KeyboardHidTracker::thread_loop()",
            [this]{ thread_loop(); }
        );
    })
{}


void GamepadTracker::stop(){
    if (!m_thread.joinable()){
        return;
    }
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_sleep_lock);
    }
    m_thread.join();
}


void GamepadTracker::clear_state(){
}

void GamepadTracker::thread_loop(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(m_logger);

    while (!m_stopping.load(std::memory_order_acquire)){
        try{
            GamepadInputState current;
            m_listeners.run_method(&ControllerInputListener::run_controller_input, current);
        }catch (Exception& e){
            e.log(global_logger_tagged());
        }

        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_stopping.load(std::memory_order_acquire)){
            return;
        }
    }
}






}
