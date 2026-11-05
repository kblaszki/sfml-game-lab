add_library(tappy_app STATIC
    src/Bindings.cpp
    src/InputMapping.cpp
    src/Sounds.cpp
    src/Feedback.cpp
    src/SpriteBank.cpp
    src/Scenes.cpp
    src/MainMenuScene.cpp
    src/PlayScene.cpp
    src/PauseScene.cpp
    src/GameOverScene.cpp
)
target_include_directories(tappy_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(tappy_app PUBLIC cxx_std_23)
target_compile_options(tappy_app PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(tappy_app PUBLIC
    tappy_sim
    sgl_scene
    sgl_render
    sgl_input
    sgl_ui
    sgl_loop
    sgl_resources
    sgl_audio
    sgl_save
)
