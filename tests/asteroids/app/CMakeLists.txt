add_unit_test(asteroids_scenes_test
    SOURCES
        ScenesTest.cpp
    DEPENDENCIES
        asteroids_app
)

add_unit_test(asteroids_play_scene_test
    SOURCES
        PlaySceneTest.cpp
    DEPENDENCIES
        asteroids_app
)

add_unit_test(asteroids_sounds_test
    SOURCES
        SoundsTest.cpp
    DEPENDENCIES
        asteroids_app
)

add_unit_test(asteroids_feedback_test
    SOURCES
        FeedbackTest.cpp
    DEPENDENCIES
        asteroids_app
)
