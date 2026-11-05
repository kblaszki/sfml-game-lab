add_unit_test(tappy_scenes_test
    SOURCES
        ScenesTest.cpp
    DEPENDENCIES
        tappy_app
)

add_unit_test(tappy_play_scene_test
    SOURCES
        PlaySceneTest.cpp
    DEPENDENCIES
        tappy_app
)

add_unit_test(tappy_sounds_test
    SOURCES
        SoundsTest.cpp
    DEPENDENCIES
        tappy_app
)

add_unit_test(tappy_feedback_test
    SOURCES
        FeedbackTest.cpp
    DEPENDENCIES
        tappy_app
)
