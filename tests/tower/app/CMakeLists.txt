add_unit_test(tower_scenes_test
    SOURCES
        ScenesTest.cpp
    DEPENDENCIES
        tower_app
)

add_unit_test(tower_play_scene_test
    SOURCES
        PlaySceneTest.cpp
    DEPENDENCIES
        tower_app
)

add_unit_test(tower_sounds_test
    SOURCES
        SoundsTest.cpp
    DEPENDENCIES
        tower_app
)

add_unit_test(tower_feedback_test
    SOURCES
        FeedbackTest.cpp
    DEPENDENCIES
        tower_app
)

add_unit_test(tower_sprite_bank_test
    SOURCES
        SpriteBankTest.cpp
    DEPENDENCIES
        tower_app
)
