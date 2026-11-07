/*  Device Dropdown Aliases
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/Backends/AudioDeviceInfo.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#ifdef QT_SERIALPORT_LIB
#include "Controllers/SerialPort/SerialPortPollerQt.h"
#endif
#include "DeviceDropdownAliases.h"

namespace PokemonAutomation{
namespace{

const EnumDropdownDatabase<DeviceAliasType>& categories(){
    static const EnumDropdownDatabase<DeviceAliasType> database{
        {DeviceAliasType::SERIAL, "Serial", "Serial"},
        {DeviceAliasType::VIDEO, "Video", "Video"},
        {DeviceAliasType::AUDIO, "Audio", "Audio"},
    };
    return database;
}

}

DeviceAliasRow::DeviceAliasRow(EditableTableOption& parent)
    : EditableTableRow(parent)
    , category(categories(), LockMode::UNLOCK_WHILE_RUNNING, DeviceAliasType::VIDEO)
    , device_id(false, LockMode::UNLOCK_WHILE_RUNNING, "", "Device ID")
    , device_name(false, LockMode::READ_ONLY, "", "")
    , alias(false, LockMode::UNLOCK_WHILE_RUNNING, "", "Use original name")
{
    PA_ADD_OPTION(category);
    PA_ADD_OPTION(device_id);
    PA_ADD_OPTION(device_name);
    PA_ADD_OPTION(alias);
}

std::unique_ptr<EditableTableRow> DeviceAliasRow::clone() const{
    auto row = std::make_unique<DeviceAliasRow>(parent());
    row->category.set(category);
    row->device_id.set(device_id);
    row->device_name.set(device_name);
    row->alias.set(alias);
    return row;
}

DeviceDropdownAliases::DeviceDropdownAliases()
    : EditableTableOption_t<DeviceAliasRow>(
        "<b>Device Aliases:</b><br>"
        "Choose names for serial, cameras and audio devices."
        "Connected devices are added when Global Settings opens.<br>"
        "An empty alias uses the original name.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true,
        10
    )
{}

std::vector<std::string> DeviceDropdownAliases::make_header() const{
    return {"Type", "Device ID", "Original Name", "Alias"};
}

std::optional<std::string> DeviceDropdownAliases::lookup(DeviceAliasType category, const std::string& device_id) const{
    if (device_id == "COM5"){
        return "Controller 1";
    }else if (device_id == "COM8"){
        return "Controller 2";
    }else if (device_id == "COM7"){
        return "Controller 3";
    }else if (device_id == R"(\\?\usb#vid_298f&pid_1996&mi_00#6&1df70757&0&0000#{e5323777-f976-4f5b-9b55-b94699c46e44}\global)"){
        return "ShadowCast Video 1";
    }
    else if (device_id == R"(\\?\usb#vid_298f&pid_1996&mi_00#6&341e7014&0&0000#{e5323777-f976-4f5b-9b55-b94699c46e44}\global)"){
        return "ShadowCast Video 2";
    }
    else if (device_id == R"(\\?\usb#vid_298f&pid_1996&mi_00#6&b3d56b2&0&0000#{e5323777-f976-4f5b-9b55-b94699c46e44}\global)"){
        return "ShadowCast Video 3";
    }
    else if (device_id == R"({0.0.1.00000000}.{c55cb6a7-0e77-4af3-83a0-d9ab1aeeea72})"){
        return "ShadowCast Audio 1";
    }
    else if (device_id == R"({0.0.1.00000000}.{ca75b45c-674f-4c5a-90ed-db3d82a5a4c5})"){
        return "ShadowCast Audio 2";
    }
    else if (device_id == R"({0.0.1.00000000}.{f087b64e-d99f-47fe-b3b0-483b35d7fb70})"){
        return "ShadowCast Audio 3";
    }

    std::optional<std::string> result;
    EditableTableOption::run_on_all_rows<DeviceAliasRow>([&](const DeviceAliasRow& row){
        if (row.category != category || std::string(row.device_id) != device_id){
            return false;
        }
        std::string alias = row.alias;
        if (!alias.empty()){
            result = std::move(alias);
        }
        return true;
    });
    return result;
}

void DeviceDropdownAliases::add_device(DeviceAliasType category, const std::string& id, const std::string& name){
    bool found = false;
    run_on_all_rows([&](DeviceAliasRow& row){
        if (row.category != category || std::string(row.device_id) != id){
            return false;
        }
        row.device_name.set(name);
        found = true;
        return true;
    });
    if (!found){
        auto row = std::make_unique<DeviceAliasRow>(*this);
        row->category.set(category);
        row->device_id.set(id);
        row->device_name.set(name);
        append_row(std::move(row));
    }
}

void DeviceDropdownAliases::add_connected_devices(){
#ifdef QT_SERIALPORT_LIB
    for (const QSerialPortInfo& info : SerialPortPoller::instance().ports()){
        add_device(DeviceAliasType::SERIAL, info.portName().toStdString(), info.description().toStdString());
    }
#endif
    for (const CameraInfo& info : get_all_cameras()){
        add_device(DeviceAliasType::VIDEO, info.device_name(), get_camera_name(info));
    }
    for (const AudioDeviceInfo& info : AudioDeviceInfo::all_input_devices()){
        add_device(DeviceAliasType::AUDIO, info.device_name(), info.original_name());
    }
    for (const AudioDeviceInfo& info : AudioDeviceInfo::all_output_devices()){
        add_device(DeviceAliasType::AUDIO, info.device_name(), info.original_name());
    }
}

DeviceAliasOptions::DeviceAliasOptions()
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING, false)
{
    PA_ADD_OPTION(m_aliases);
}

JsonValue DeviceAliasOptions::to_json() const{
    return m_aliases.to_json();
}
void DeviceAliasOptions::load_json(const JsonValue& json){
    m_aliases.load_json(json);
}

void DeviceAliasOptions::add_connected_devices(){
    m_aliases.add_connected_devices();
}
std::optional<std::string> DeviceAliasOptions::lookup(DeviceAliasType category, const std::string& device_id) const{
    return m_aliases.lookup(category, device_id);
}

std::optional<std::string> device_dropdown_name(DeviceAliasType category, const std::string& device_id){
    return GlobalSettings::instance().DEVICE_ALIASES->lookup(category, device_id);
}

}
