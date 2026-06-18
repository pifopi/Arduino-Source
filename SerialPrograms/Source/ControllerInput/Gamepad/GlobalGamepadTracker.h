/*  Global Gamepad Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Gamepad_GlobalGamepadTracker_H
#define PokemonAutomation_ControllerInput_Gamepad_GlobalGamepadTracker_H

#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "ControllerInput/ControllerInput.h"

namespace PokemonAutomation{


class GamepadTracker final : public ControllerInputSource{
public:
    ~GamepadTracker();
    GamepadTracker();

    virtual void stop() noexcept override;

    virtual void clear_state() override;

private:
    void thread_loop();

private:
    TaggedLogger m_logger;

    std::atomic<bool> m_stopping;

    AsyncTask m_thread;
};


GamepadTracker& global_gamepad_tracker();




}
#endif
