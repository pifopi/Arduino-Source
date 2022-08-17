/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_Options_SimpleIntegerOption_H
#define PokemonAutomation_Options_SimpleIntegerOption_H

#include <limits>
#include <atomic>
#include "ConfigOption.h"

namespace PokemonAutomation{



template <typename Type>
class SimpleIntegerCell : public ConfigOption{
public:
    SimpleIntegerCell(
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    Type min_value() const{ return m_min_value; }
    Type max_value() const{ return m_max_value; }
    Type default_value() const{ return m_default; }

    operator Type() const{ return m_current.load(std::memory_order_relaxed); }
    std::string set(Type x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity(Type x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

public:
    virtual ConfigWidget* make_ui(QWidget& parent) override;

protected:
    const Type m_min_value;
    const Type m_max_value;
    const Type m_default;
    std::atomic<Type> m_current;
};





template <typename Type>
class SimpleIntegerOption : public SimpleIntegerCell<Type>{
public:
    SimpleIntegerOption(
        std::string label,
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const{ return m_label; }

public:
    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
};




}
#endif
