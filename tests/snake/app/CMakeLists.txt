add_unit_test(snake_scenes_test
    SOURCES
        ScenesTest.cpp
    DEPENDENCIES
        snake_app
)

add_unit_test(snake_play_scene_test
    SOURCES
        PlaySceneTest.cpp
    DEPENDENCIES
        snake_app
)
