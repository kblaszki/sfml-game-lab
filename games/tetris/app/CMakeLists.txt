add_library(tetris_app STATIC
    src/Bindings.cpp
    src/InputMapping.cpp
    src/BoardRender.cpp
    src/HudModel.cpp
    src/Sounds.cpp
    src/Feedback.cpp
    src/Scenes.cpp
    src/MainMenuScene.cpp
    src/PlayScene.cpp
    src/PauseScene.cpp
    src/GameOverScene.cpp
)
target_include_directories(tetris_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(tetris_app PUBLIC cxx_std_23)
target_compile_options(tetris_app PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(tetris_app PUBLIC
    tetris_sim
    sgl_loop
    sgl_resources
    sgl_audio
    sgl_fx
    sgl_save
    sgl_ui
)
