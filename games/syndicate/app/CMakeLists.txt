add_library(syndicate_app STATIC
    src/Theme.cpp
    src/LayoutView.cpp
    src/BoardRender.cpp
    src/PawnAnimator.cpp
    src/Bindings.cpp
    src/HudModel.cpp
    src/PlayScene.cpp
    src/Sounds.cpp
    src/Feedback.cpp
    src/Scenes.cpp
    src/MainMenuScene.cpp
    src/SetupScene.cpp
    src/GameOverScene.cpp
)
target_include_directories(syndicate_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(syndicate_app PUBLIC cxx_std_23)
target_compile_options(syndicate_app PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(syndicate_app PUBLIC
    syndicate_sim
    sgl_render
    sgl_scene
    sgl_core
    sgl_input
    sgl_ui
    sgl_audio
    sgl_fx
    sgl_save
)
