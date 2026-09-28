cmake_minimum_required(VERSION 3.20)

project(sfml-game-lab VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND
        BASE_COMPILE_FLAGS
        "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall;-Wextra;-pedantic;-Werror>"
        "$<$<CXX_COMPILER_ID:MSVC>:/W4;/WX>")

option(SGL_ENABLE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(SGL_ENABLE_COVERAGE "Enable GCC coverage instrumentation (--coverage)" OFF)

if(SGL_ENABLE_ASAN)
    if(MSVC)
        message(STATUS "SGL_ENABLE_ASAN: skipped on MSVC (preset intended for GCC/Clang)")
    else()
        list(APPEND
                BASE_COMPILE_FLAGS
                "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fsanitize=address,undefined;-fno-omit-frame-pointer>")
        add_link_options(
            "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fsanitize=address,undefined>")
    endif()
endif()

if(SGL_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND BASE_COMPILE_FLAGS "--coverage")
        add_link_options(--coverage)
    else()
        message(STATUS "SGL_ENABLE_COVERAGE: requires GCC; skipping for ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

include(FetchSFML)

add_custom_target(
    check_boundaries
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/CheckBoundaries.cmake
    COMMENT "Checking engine/games boundary includes"
    VERBATIM
)

add_subdirectory(engine)
add_subdirectory(games)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND
            BASE_COMPILE_FLAGS
            "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-O3>"
            "$<$<CXX_COMPILER_ID:MSVC>:/Ox>")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND
            BASE_COMPILE_FLAGS
           "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-g>"
            "$<$<CXX_COMPILER_ID:MSVC>:/Zi>")

    file(GLOB_RECURSE FORMAT_SOURCES
        "engine/*.cpp" "engine/*.hpp"
        "games/*.cpp" "games/*.hpp"
        "tests/*.cpp" "tests/*.hpp")

    add_custom_target(
        format
        COMMAND clang-format -i ${FORMAT_SOURCES}
    )

    find_program(SGL_RUN_CLANG_TIDY NAMES run-clang-tidy run-clang-tidy.py)
    find_package(Python3 COMPONENTS Interpreter)
    if(SGL_RUN_CLANG_TIDY AND Python3_Interpreter_FOUND)
        # Match only repo engine/ and games/ TUs (not tests/*/engine paths).
        file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" SGL_SOURCE_DIR_CMAKE)
        string(REPLACE "/" "[/\\\\]" SGL_SOURCE_DIR_RE "${SGL_SOURCE_DIR_CMAKE}")
        set(SGL_TIDY_SOURCE_FILTER "${SGL_SOURCE_DIR_RE}[/\\\\](engine|games)[/\\\\].*\\.(cpp|cc|cxx)$")
        set(SGL_TIDY_HEADER_FILTER "${SGL_SOURCE_DIR_RE}[/\\\\](engine|games)[/\\\\].*")

        set(SGL_TIDY_EXTRA_ARGS "-extra-arg=-Wno-unknown-warning-option")
        # clang-tidy needs explicit libstdc++ paths when the compile DB was produced by GCC.
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            # libstdc++ 14 exposes std::expected only when __cpp_concepts >= 202002.
            # clang-tidy-18 keeps that macro at 201907 even with -std=c++23.
            list(APPEND SGL_TIDY_EXTRA_ARGS
                "-extra-arg=-std=c++23"
                "-extra-arg=-D__cpp_concepts=202002L")
            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=include
                OUTPUT_VARIABLE SGL_GCC_INCLUDE
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                OUTPUT_VARIABLE SGL_GCC_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
            get_filename_component(SGL_GCC_PREFIX "${CMAKE_CXX_COMPILER}" DIRECTORY)
            get_filename_component(SGL_GCC_PREFIX "${SGL_GCC_PREFIX}" DIRECTORY)
            set(SGL_GCC_CXX_INC "${SGL_GCC_PREFIX}/include/c++/${SGL_GCC_VERSION}")
            if(EXISTS "${SGL_GCC_CXX_INC}")
                list(APPEND SGL_TIDY_EXTRA_ARGS
                    "-extra-arg=-isystem${SGL_GCC_CXX_INC}")
                file(GLOB SGL_GCC_TARGET_INCS "${SGL_GCC_CXX_INC}/*-*-*")
                foreach(SGL_GCC_TARGET_INC IN LISTS SGL_GCC_TARGET_INCS)
                    if(IS_DIRECTORY "${SGL_GCC_TARGET_INC}")
                        list(APPEND SGL_TIDY_EXTRA_ARGS
                            "-extra-arg=-isystem${SGL_GCC_TARGET_INC}")
                    endif()
                endforeach()
            endif()
            if(EXISTS "${SGL_GCC_PREFIX}/include")
                list(APPEND SGL_TIDY_EXTRA_ARGS
                    "-extra-arg=-isystem${SGL_GCC_PREFIX}/include")
            endif()
            if(SGL_GCC_INCLUDE AND EXISTS "${SGL_GCC_INCLUDE}")
                list(APPEND SGL_TIDY_EXTRA_ARGS
                    "-extra-arg=-isystem${SGL_GCC_INCLUDE}")
            endif()
        endif()

        set(SGL_TIDY_SOURCE_ARG)
        set(SGL_TIDY_FILES)
        execute_process(
            COMMAND ${Python3_EXECUTABLE} ${SGL_RUN_CLANG_TIDY} -h
            OUTPUT_VARIABLE SGL_TIDY_HELP
            ERROR_VARIABLE SGL_TIDY_HELP_ERR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE)
        if("${SGL_TIDY_HELP}${SGL_TIDY_HELP_ERR}" MATCHES "source-filter")
            set(SGL_TIDY_SOURCE_ARG "-source-filter=${SGL_TIDY_SOURCE_FILTER}")
        else()
            file(GLOB_RECURSE SGL_TIDY_FILES CONFIGURE_DEPENDS
                "${CMAKE_SOURCE_DIR}/engine/*.cpp"
                "${CMAKE_SOURCE_DIR}/engine/*.cc"
                "${CMAKE_SOURCE_DIR}/engine/*.cxx"
                "${CMAKE_SOURCE_DIR}/games/*.cpp"
                "${CMAKE_SOURCE_DIR}/games/*.cc"
                "${CMAKE_SOURCE_DIR}/games/*.cxx")
        endif()

        add_custom_target(
            tidy
            COMMAND ${Python3_EXECUTABLE} ${SGL_RUN_CLANG_TIDY}
                -p ${CMAKE_BINARY_DIR}
                -quiet
                "-header-filter=${SGL_TIDY_HEADER_FILTER}"
                ${SGL_TIDY_SOURCE_ARG}
                ${SGL_TIDY_EXTRA_ARGS}
                ${SGL_TIDY_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy on engine/ and games/"
            VERBATIM
        )
    else()
        add_custom_target(
            tidy
            COMMAND ${CMAKE_COMMAND} -E echo "run-clang-tidy and Python3 required; install clang-tools"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "tidy requires run-clang-tidy"
            VERBATIM
        )
    endif()

    include(CTest)
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()
