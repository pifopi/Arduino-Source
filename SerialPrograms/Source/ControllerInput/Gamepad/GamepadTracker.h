/*  Gamepad Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Gamepad_GamepadTracker_H
#define PokemonAutomation_ControllerInput_Gamepad_GamepadTracker_H

#include <mutex>
#include "Common/Cpp/Concurrency/Thread.h"
#include "CommonFramework/Logging/Logger.h"
#include "ControllerInput/ControllerInput.h"

class QKeyEvent;

namespace PokemonAutomation{


class GamepadTracker final : public ControllerInputSource{
public:
    ~GamepadTracker();
    GamepadTracker();

    virtual void stop() override;

    virtual void clear_state() override;

private:
    void thread_loop();

private:
    TaggedLogger m_logger;

    std::atomic<bool> m_stopping;

    std::mutex m_sleep_lock;
    Thread m_thread;
};


GamepadTracker& global_gamepad_tracker();




}
#endif
