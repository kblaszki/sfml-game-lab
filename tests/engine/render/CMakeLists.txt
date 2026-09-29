add_unit_test(render_queue_test
    SOURCES
        RenderQueueTest.cpp
    DEPENDENCIES
        sgl_render
)

add_unit_test(projection_test
    SOURCES
        ProjectionTest.cpp
    DEPENDENCIES
        sgl_render
)

add_unit_test(shapes_test
    SOURCES
        ShapesTest.cpp
    DEPENDENCIES
        sgl_render
)
