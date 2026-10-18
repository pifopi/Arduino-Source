# SFML supplies raw joystick input on Windows, macOS and Linux.
option(PA_GAMEPAD "Enable physical gamepad input" ON)
if(PA_GAMEPAD)
    include(${CMAKE_CURRENT_LIST_DIR}/SFML.cmake)
    target_sources(SerialProgramsLib PRIVATE
        Source/ControllerInput/Gamepad/GlobalGamepadTracker.cpp
        Source/ControllerInput/Gamepad/GlobalGamepadTracker.h
        Source/ControllerInput/Gamepad/GamepadInput_State.h
        Source/ControllerInput/Gamepad/GamepadInput_SFML.h
        Source/ControllerInput/Gamepad/GamepadInput_Mapping.cpp
        Source/ControllerInput/Gamepad/GamepadInput_Mapping.h
        Source/NintendoSwitch/Controllers/NintendoSwitch_Gamepad.h
    )
    target_compile_definitions(SerialProgramsLib PRIVATE PA_GAMEPAD)
    target_compile_definitions(SerialPrograms PRIVATE PA_GAMEPAD)
    target_link_libraries(SerialProgramsLib PRIVATE SFML::Window)
    add_custom_command(TARGET SerialPrograms POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/../../3rdParty/SFML-LICENSE.txt"
            "${DEST_DIR}/SFML-LICENSE.txt"
    )
endif()
