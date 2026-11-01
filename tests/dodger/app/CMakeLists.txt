add_unit_test(dodger_scenes_test
    SOURCES
        ScenesTest.cpp
    DEPENDENCIES
        dodger_app
)

add_unit_test(dodger_play_scene_test
    SOURCES
        PlaySceneTest.cpp
    DEPENDENCIES
        dodger_app
)

add_unit_test(dodger_sounds_test
    SOURCES
        SoundsTest.cpp
    DEPENDENCIES
        dodger_app
)

add_unit_test(dodger_feedback_test
    SOURCES
        FeedbackTest.cpp
    DEPENDENCIES
        dodger_app
)
