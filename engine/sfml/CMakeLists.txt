add_library(sgl_sfml STATIC
    src/SfmlClock.cpp
    src/SfmlAssets.cpp
    src/SfmlAudio.cpp
    src/SfmlRenderer.cpp
    src/SfmlPlatform.cpp
)
target_include_directories(sgl_sfml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(sgl_sfml PUBLIC cxx_std_23)
target_compile_options(sgl_sfml PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(sgl_sfml
    PUBLIC
        sgl_loop
        sgl_resources
        sgl_audio
        sgl_save
        SFML::Graphics
        SFML::Window
        SFML::System
    PRIVATE
        SFML::Audio
)
