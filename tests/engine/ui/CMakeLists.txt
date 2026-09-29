add_unit_test(menu_model_test
    SOURCES
        MenuModelTest.cpp
    DEPENDENCIES
        sgl_ui
)

add_unit_test(pause_overlay_test
    SOURCES
        PauseOverlayTest.cpp
        ../scene/fakes/SceneSpy.cpp
    DEPENDENCIES
        sgl_ui
)

add_unit_test(settings_scene_test
    SOURCES
        SettingsSceneTest.cpp
        ../scene/fakes/SceneSpy.cpp
    DEPENDENCIES
        sgl_ui
)

add_unit_test(text_layout_test
    SOURCES
        TextLayoutTest.cpp
    DEPENDENCIES
        sgl_ui
)

add_unit_test(tooltip_test
    SOURCES
        TooltipTest.cpp
    DEPENDENCIES
        sgl_ui
)

add_unit_test(hover_anim_test
    SOURCES
        HoverAnimTest.cpp
    DEPENDENCIES
        sgl_ui
)
