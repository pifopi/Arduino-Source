/*  Global Gamepad Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "GamepadInput_State.h"
#include "GlobalGamepadTracker.h"

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
    , m_thread(
        GlobalThreadPools::unlimited_realtime().dispatch_now_blocking([this]{
            run_with_catch(
                "GamepadTracker::thread_loop()",
                [this]{ thread_loop(); }
            );
        })
    )
{}


void GamepadTracker::stop() noexcept{
    if (!m_thread){
        return;
    }
    m_stopping.store(true, std::memory_order_release);
}


void GamepadTracker::clear_state(){
    // We don't need to do anything as controllers should control PA software even when not focused
}


void GamepadTracker::thread_loop(){
    //GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(m_logger);

    //GamepadInputState last;
    //GamepadInputState current;

    //bool last_neutral = true;
    //WallClock last_press = current_time();
    //while (!m_stopping.load(std::memory_order_acquire)){
    //    //TODO gdottel fill
    //}
}




}
