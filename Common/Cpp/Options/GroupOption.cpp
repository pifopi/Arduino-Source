/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include "Common/Compiler.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "GroupOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct GroupOption::Data{
    const std::string m_label;
    const bool m_toggleable;
    const bool m_default_enabled;
    std::atomic<bool> m_enabled;

    Data(
        std::string label,
        bool toggleable,
        bool enabled
    )
        : m_label(std::move(label))
        , m_toggleable(toggleable)
        , m_default_enabled(enabled)
        , m_enabled(enabled)
    {}
};


GroupOption::~GroupOption() = default;
GroupOption::GroupOption(
    std::string label,
    LockMode lock_while_program_is_running,
    bool toggleable,
    bool enabled
)
    : BatchOption(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, std::move(label), toggleable, enabled)
{}

const std::string GroupOption::label() const{
    return m_data->m_label;
}
bool GroupOption::toggleable() const{
    return m_data->m_toggleable;
}
bool GroupOption::enabled() const{
    return m_data->m_enabled.load(std::memory_order_relaxed);
}
void GroupOption::set_enabled(bool enabled){
    if (enabled != m_data->m_enabled.exchange(enabled, std::memory_order_relaxed)){
        report_value_changed(this);
        on_set_enabled(enabled);
    }
}
void GroupOption::load_json(const JsonValue& json){
    BatchOption::load_json(json);
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    if (m_data->m_toggleable){
        bool enabled;
        if (obj->read_boolean(enabled, "Enabled")){
            if (enabled != m_data->m_enabled.exchange(enabled, std::memory_order_relaxed)){
                report_value_changed(this);
            }
            on_set_enabled(enabled);
        }
    }
}
JsonValue GroupOption::to_json() const{
    JsonObject obj = std::move(*BatchOption::to_json().to_object());
    if (m_data->m_toggleable){
        obj["Enabled"] = m_data->m_enabled.load(std::memory_order_relaxed);
    }
    return obj;
}
void GroupOption::restore_defaults(){
    BatchOption::restore_defaults();
    bool default_value = m_data->m_default_enabled;
    if (default_value != m_data->m_enabled.exchange(default_value, std::memory_order_relaxed)){
        report_value_changed(this);
        on_set_enabled(default_value);
    }
}
void GroupOption::on_set_enabled(bool enabled){}





}
