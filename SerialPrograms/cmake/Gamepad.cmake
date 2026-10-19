# SDL supplies standardized gamepad mappings on Windows, macOS and Linux.
# Prefer an installed static library, otherwise build a pinned input-only SDL.
option(PA_GAMEPAD "Enable physical gamepad input" ON)
if(PA_GAMEPAD)
    include(${CMAKE_CURRENT_LIST_DIR}/SDL3.cmake)
    target_sources(SerialProgramsLib PRIVATE
        Source/ControllerInput/Gamepad/GlobalGamepadTracker.cpp
        Source/ControllerInput/Gamepad/GlobalGamepadTracker.h
        Source/ControllerInput/Gamepad/GamepadInput_State.h
        Source/ControllerInput/Gamepad/GamepadInput_SDL.h
        Source/NintendoSwitch/Controllers/NintendoSwitch_Gamepad.h
    )
    target_compile_definitions(SerialProgramsLib PRIVATE PA_GAMEPAD)
    target_compile_definitions(SerialPrograms PRIVATE PA_GAMEPAD)
    target_link_libraries(SerialProgramsLib PRIVATE SDL3::SDL3-static)
    add_custom_command(TARGET SerialPrograms POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/../../3rdParty/SDL3-LICENSE.txt"
            "${DEST_DIR}/SDL3-LICENSE.txt"
    )
endif()

