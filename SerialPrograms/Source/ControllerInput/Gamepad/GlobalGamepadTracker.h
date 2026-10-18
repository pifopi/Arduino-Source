#ifndef PokemonAutomation_GlobalGamepadTracker_H
#define PokemonAutomation_GlobalGamepadTracker_H

#include <atomic>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "ControllerInput/ControllerInput.h"
#include "GamepadInput_Mapping.h"

namespace PokemonAutomation{

class GamepadTracker final : public ControllerInputSource{
public:
    ~GamepadTracker(){ stop(); }
    // Load mapping and start polling. Qt remains the only window/event owner.
    void start();
    void stop() noexcept override;
    void clear_state() override{ m_reset.store(true, std::memory_order_release); }
private:
    void thread_loop();
    bool m_initialized = false;
    GamepadMapping m_mapping;
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_reset{true};
    Mutex m_lock;
    ConditionVariable m_cv;
    AsyncTask m_thread;
};

GamepadTracker& global_gamepad_tracker();
}
#endif
