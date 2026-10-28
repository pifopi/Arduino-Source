function(pa_find_sfml)
    set(SFML_STATIC_LIBRARIES TRUE)
    find_package(SFML 3.0 CONFIG QUIET COMPONENTS Window System)
    if(NOT TARGET SFML::Window)
        include(FetchContent)
        # Function scope preserves the application's own shared-library options.
        set(BUILD_SHARED_LIBS OFF)
        set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
        foreach(module GRAPHICS AUDIO NETWORK EXAMPLES TEST_SUITE)
            set(SFML_BUILD_${module} OFF CACHE BOOL "" FORCE)
        endforeach()
        set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
        set(CMAKE_AUTOMOC OFF)
        set(CMAKE_AUTORCC OFF)
        set(CMAKE_AUTOUIC OFF)
        FetchContent_Declare(SFML
            URL https://www.sfml-dev.org/files/SFML-3.0.2-sources.zip
            URL_HASH SHA256=3c8467414cf634c32419973b22faefcb8b30002f908b77fb658692e9a580a907
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            TIMEOUT 60
        )
        FetchContent_MakeAvailable(SFML)
    endif()
endfunction()
pa_find_sfml()
