if(WIN32)
    set(_sfml_headers "${CMAKE_CURRENT_LIST_DIR}/../../3rdParty/SFML-3.1.0")
    set(_sfml_binaries "${CMAKE_CURRENT_LIST_DIR}/../../3rdPartyBinaries")

    foreach(module System Window)
        string(TOLOWER "${module}" module_name)
        # Keep the DLLs zipped in Git, like OpenCV. The application's existing
        # third-party DLL deployment step copies the extracted files.
        foreach(suffix "" "-d")
            set(dll "${_sfml_binaries}/sfml-${module_name}${suffix}-3.dll")
            if(NOT EXISTS "${dll}" OR "${_sfml_binaries}/sfml-${module_name}${suffix}-3.zip" IS_NEWER_THAN "${dll}")
                file(ARCHIVE_EXTRACT
                    INPUT "${_sfml_binaries}/sfml-${module_name}${suffix}-3.zip"
                    DESTINATION "${_sfml_binaries}"
                )
                file(TOUCH "${dll}")
            endif()
        endforeach()

        add_library(SFML::${module} SHARED IMPORTED)
        set_target_properties(SFML::${module} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_sfml_headers}"
            IMPORTED_CONFIGURATIONS "Debug;Release"
            IMPORTED_IMPLIB_RELEASE "${_sfml_binaries}/sfml-${module_name}.lib"
            IMPORTED_IMPLIB_DEBUG "${_sfml_binaries}/sfml-${module_name}-d.lib"
            IMPORTED_LOCATION_RELEASE "${_sfml_binaries}/sfml-${module_name}-3.dll"
            IMPORTED_LOCATION_DEBUG "${_sfml_binaries}/sfml-${module_name}-d-3.dll"
            MAP_IMPORTED_CONFIG_DEBUG DEBUG
            MAP_IMPORTED_CONFIG_RELEASE RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
        )
    endforeach()
    set_target_properties(SFML::Window PROPERTIES INTERFACE_LINK_LIBRARIES SFML::System)
else()
    find_package(SFML 3.1 CONFIG REQUIRED COMPONENTS Window System)
endif()
