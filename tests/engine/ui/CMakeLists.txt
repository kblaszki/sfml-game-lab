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
