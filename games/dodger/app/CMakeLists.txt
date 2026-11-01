add_library(dodger_app STATIC
    src/Bindings.cpp
    src/InputMapping.cpp
    src/Scenes.cpp
    src/MainMenuScene.cpp
    src/PlayScene.cpp
    src/PauseScene.cpp
    src/GameOverScene.cpp
)
target_include_directories(dodger_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(dodger_app PUBLIC cxx_std_23)
target_compile_options(dodger_app PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(dodger_app PUBLIC
    dodger_sim
    sgl_scene
    sgl_render
    sgl_input
    sgl_ui
    sgl_loop
)
