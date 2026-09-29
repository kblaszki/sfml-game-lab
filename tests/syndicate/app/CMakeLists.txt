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
