#ifndef PokemonAutomation_GlobalGamepadTracker_H
#define PokemonAutomation_GlobalGamepadTracker_H

#include <atomic>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "ControllerInput/ControllerInput.h"

namespace PokemonAutomation{

class GamepadTracker final : public ControllerInputSource{
public:
    ~GamepadTracker(){ stop(); }
    // Initialize and shut down SDL on the main thread, while QApplication lives.
    void start();
    void stop() noexcept override;
    void clear_state() override{ m_reset.store(true, std::memory_order_release); }
private:
    void thread_loop();
    bool m_initialized = false;
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_reset{true};
    Mutex m_lock;
    ConditionVariable m_cv;
    AsyncTask m_thread;
};

GamepadTracker& global_gamepad_tracker();
}
#endif
