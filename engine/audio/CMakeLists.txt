add_library(sgl_audio STATIC
    src/AudioI.cpp
    src/NullAudio.cpp
    src/Tone.cpp
    src/Blip.cpp
)
target_include_directories(sgl_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(sgl_audio PUBLIC cxx_std_23)
target_compile_options(sgl_audio PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(sgl_audio PUBLIC sgl_core)
