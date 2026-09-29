add_unit_test(syndicate_layout_view_test
    SOURCES
        LayoutViewTest.cpp
    DEPENDENCIES
        syndicate_app
)

add_unit_test(syndicate_board_render_test
    SOURCES
        BoardRenderTest.cpp
    DEPENDENCIES
        syndicate_app
)

add_unit_test(syndicate_pawn_animator_test
    SOURCES
        PawnAnimatorTest.cpp
    DEPENDENCIES
        syndicate_app
)

add_unit_test(syndicate_hud_model_test
    SOURCES
        HudModelTest.cpp
    DEPENDENCIES
        syndicate_app
)

add_unit_test(syndicate_play_scene_test
    SOURCES
        PlaySceneTest.cpp
    DEPENDENCIES
        syndicate_app
)

add_unit_test(syndicate_scenes_test
    SOURCES
        ScenesTest.cpp
    DEPENDENCIES
        syndicate_app
)

add_unit_test(syndicate_feedback_test
    SOURCES
        FeedbackTest.cpp
    DEPENDENCIES
        syndicate_app
)
