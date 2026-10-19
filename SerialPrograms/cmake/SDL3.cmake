function(pa_find_sdl3)
    find_package(SDL3 3.2 CONFIG QUIET COMPONENTS SDL3-static)
    if(NOT TARGET SDL3::SDL3-static)
        include(FetchContent)
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        set(SDL_TESTS OFF CACHE BOOL "" FORCE)
        set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
        foreach(subsystem AUDIO VIDEO GPU RENDER CAMERA HAPTIC SENSOR DIALOG TRAY)
            set(SDL_${subsystem} OFF CACHE BOOL "" FORCE)
        endforeach()
        FetchContent_Declare(SDL3
            URL https://www.libsdl.org/release/SDL3-3.2.10.tar.gz
            URL_HASH SHA256=f87be7b4dec66db4098e9c167b2aa34e2ca10aeb5443bdde95ae03185ed513e0
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            TIMEOUT 60
        )
        # SDL does not use Qt's automatic source processing.
        set(CMAKE_AUTOMOC OFF)
        set(CMAKE_AUTORCC OFF)
        set(CMAKE_AUTOUIC OFF)
        FetchContent_MakeAvailable(SDL3)
    endif()
endfunction()
pa_find_sdl3()

