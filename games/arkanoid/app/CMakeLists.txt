add_library(arkanoid_app STATIC
    src/Assets.cpp
    src/Bindings.cpp
    src/HudModel.cpp
    src/SceneRender.cpp
    src/Sounds.cpp
    src/Feedback.cpp
    src/Scenes.cpp
    src/MainMenuScene.cpp
    src/GameplayScene.cpp
    src/PauseScene.cpp
    src/ResultScene.cpp
)
target_include_directories(arkanoid_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(arkanoid_app PUBLIC cxx_std_23)
target_compile_options(arkanoid_app PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(arkanoid_app PUBLIC
    arkanoid_sim
    sgl_loop
    sgl_resources
    sgl_audio
    sgl_fx
    sgl_save
    sgl_ui
)
