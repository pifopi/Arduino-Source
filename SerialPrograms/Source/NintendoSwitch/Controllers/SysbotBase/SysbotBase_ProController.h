/*  Nintendo Switch Pro Controller (SysbotBase)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H
#define PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H

#include <condition_variable>
#include "Common/Cpp/Containers/CircularBuffer.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerWithScheduler.h"
#include "SysbotBase_Connection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class ProController_SysbotBase final :
    public ProController,
    public ControllerWithScheduler
{
public:
    using ContextType = ProControllerContext;

    static constexpr size_t QUEUE_SIZE = 4;


public:
    ProController_SysbotBase(
        Logger& logger,
        SysbotBase::TcpSysbotBase_Connection& connection
    );
    ~ProController_SysbotBase();
    void stop();


public:
    virtual ControllerType controller_type() const override{
        return ControllerType::NintendoSwitch_WiredController;
    }
    virtual const ControllerFeatures& controller_features() const override;
    virtual ControllerPerformanceClass performance_class() const override{
        return ControllerPerformanceClass::SysbotBase;
    }

    virtual Milliseconds ticksize() const override{
        return Milliseconds::zero();
    }
    virtual Milliseconds cooldown() const override{
        return Milliseconds(150);
    }
    virtual Milliseconds timing_variation() const override{
        return ConsoleSettings::instance().TIMING_OPTIONS.SYSBOTBASE;
    }
    virtual bool atomic_multibutton() const override{
        return false;
    }


public:
    virtual Logger& logger() override{
        return m_logger;
    }
    virtual RecursiveThrottler& logging_throttler() override{
        return m_logging_throttler;
    }
    virtual bool is_ready() const override{
        return m_connection.is_ready();
    }


public:
    virtual void cancel_all_commands() override;
    virtual void replace_on_next_command() override;

    virtual void wait_for_all(const Cancellable* cancellable) override;


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void issue_barrier(const Cancellable* cancellable) override{
        ControllerWithScheduler::issue_barrier(cancellable);
    }
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) override{
        ControllerWithScheduler::issue_nop(cancellable, duration);
    }

    virtual void issue_buttons(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        Button button
    ) override{
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
    }
    virtual void issue_dpad(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition position
    ) override{
        ControllerWithScheduler::issue_dpad(cancellable, delay, hold, cooldown, position);
    }
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    ) override{
        ControllerWithScheduler::issue_left_joystick(cancellable, delay, hold, cooldown, x, y);
    }
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    ) override{
        ControllerWithScheduler::issue_right_joystick(cancellable, delay, hold, cooldown, x, y);
    }

    virtual void issue_gyro_accel_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_x(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_accel_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_y(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_accel_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_z(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_x(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_y(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_z(cancellable, delay, hold, cooldown, value);
    }

    virtual void issue_full_controller_state(
        const Cancellable* cancellable,
        Milliseconds hold,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y
    ) override{
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            hold,
            button,
            position,
            left_x, left_y,
            right_x, right_y
        );
    }


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button);
    }
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button0, Button button1
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button0, button1);
    }
    virtual void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    ) override{
        ControllerWithScheduler::issue_mash_AZs(cancellable, duration);
    }
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition direction  //  Diagonals not allowed.
    ) override{
        ControllerWithScheduler::issue_system_scroll(cancellable, delay, hold, cooldown, direction);
    }


private:
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;

    void send_diff(
        const ProControllerState& old_state,
        const ProControllerState& new_state
    );
    void thread_body();


private:
    SysbotBase::TcpSysbotBase_Connection& m_connection;

    std::atomic<bool> m_stopping;
    bool m_replace_on_next;

    struct Command{
        ProControllerState state;
        Milliseconds duration;
    };
    CircularBuffer<Command> m_command_queue;

    //  WallClock::max() means the queue is empty.
    //  WallClock::min() means the state has suddently changed.
    WallClock m_next_state_change;

    std::condition_variable m_cv;
    std::thread m_dispatch_thread;
};




}
}
#endif
