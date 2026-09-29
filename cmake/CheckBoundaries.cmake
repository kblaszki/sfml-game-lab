# Usage: cmake -DSOURCE_DIR=<repo root> -P cmake/CheckBoundaries.cmake
#
# Fails if:
# 1) any file under engine/ except engine/sfml includes SFML (<SFML/ or "SFML/)
# 2) any file under games/ except games/<game>/main.cpp includes SFML (<SFML/ or "SFML/)
# 3) any file under engine/ includes a game header <name/> for a directory under games/

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "CheckBoundaries: SOURCE_DIR must be set")
endif()

file(GLOB_RECURSE ENGINE_FILES
    "${SOURCE_DIR}/engine/*.cpp"
    "${SOURCE_DIR}/engine/*.hpp"
    "${SOURCE_DIR}/engine/*.h"
    "${SOURCE_DIR}/engine/*.c"
    "${SOURCE_DIR}/engine/*.cc"
    "${SOURCE_DIR}/engine/*.cxx"
    "${SOURCE_DIR}/engine/*.ipp"
    "${SOURCE_DIR}/engine/*.inl"
)

file(GLOB_RECURSE GAMES_FILES
    "${SOURCE_DIR}/games/*.cpp"
    "${SOURCE_DIR}/games/*.hpp"
    "${SOURCE_DIR}/games/*.h"
    "${SOURCE_DIR}/games/*.c"
    "${SOURCE_DIR}/games/*.cc"
    "${SOURCE_DIR}/games/*.cxx"
    "${SOURCE_DIR}/games/*.ipp"
    "${SOURCE_DIR}/games/*.inl"
)

set(_violations "")

file(GLOB _game_entries LIST_DIRECTORIES true "${SOURCE_DIR}/games/*")
set(_game_names "")
foreach(_entry IN LISTS _game_entries)
    if(IS_DIRECTORY "${_entry}")
        get_filename_component(_game_name "${_entry}" NAME)
        list(APPEND _game_names "${_game_name}")
    endif()
endforeach()

foreach(_file IN LISTS ENGINE_FILES)
    file(TO_CMAKE_PATH "${_file}" _norm)
    string(FIND "${_norm}" "/engine/sfml/" _sfml_pos)
    set(_in_sfml FALSE)
    if(NOT _sfml_pos EQUAL -1)
        set(_in_sfml TRUE)
    endif()

    file(READ "${_file}" _contents)

    foreach(_game_name IN LISTS _game_names)
        string(FIND "${_contents}" "#include <${_game_name}/" _game_inc)
        if(NOT _game_inc EQUAL -1)
            list(APPEND _violations
                "engine must not include game headers <${_game_name}/>: ${_file}")
        endif()
    endforeach()

    if(NOT _in_sfml)
        string(FIND "${_contents}" "#include <SFML/" _inc_angle)
        string(FIND "${_contents}" "#include \"SFML/" _inc_quote)
        if(NOT _inc_angle EQUAL -1 OR NOT _inc_quote EQUAL -1)
            list(APPEND _violations
                "engine (except sfml) must not include SFML: ${_file}")
        endif()
    endif()
endforeach()

foreach(_file IN LISTS GAMES_FILES)
    file(TO_CMAKE_PATH "${_file}" _norm)
    set(_is_game_main FALSE)
    foreach(_game_name IN LISTS _game_names)
        string(FIND "${_norm}" "/games/${_game_name}/main.cpp" _main_pos)
        if(NOT _main_pos EQUAL -1)
            set(_is_game_main TRUE)
        endif()
    endforeach()
    if(_is_game_main)
        continue()
    endif()

    file(READ "${_file}" _contents)
    string(FIND "${_contents}" "#include <SFML/" _inc_angle)
    string(FIND "${_contents}" "#include \"SFML/" _inc_quote)
    if(NOT _inc_angle EQUAL -1 OR NOT _inc_quote EQUAL -1)
        list(APPEND _violations
            "games (except <game>/main.cpp) must not include SFML: ${_file}")
    endif()
endforeach()

if(_violations)
    string(JOIN "\n" _msg ${_violations})
    message(FATAL_ERROR "Boundary check failed:\n${_msg}")
endif()

message(STATUS "CheckBoundaries: OK")
