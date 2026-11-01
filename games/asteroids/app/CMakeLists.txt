add_library(asteroids_app STATIC
    src/Bindings.cpp
    src/InputMapping.cpp
    src/Sounds.cpp
    src/Feedback.cpp
    src/Scenes.cpp
    src/MainMenuScene.cpp
    src/PlayScene.cpp
    src/PauseScene.cpp
    src/GameOverScene.cpp
)
target_include_directories(asteroids_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(asteroids_app PUBLIC cxx_std_23)
target_compile_options(asteroids_app PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(asteroids_app PUBLIC
    asteroids_sim
    sgl_scene
    sgl_render
    sgl_input
    sgl_ui
    sgl_loop
    sgl_resources
    sgl_audio
    sgl_save
)
