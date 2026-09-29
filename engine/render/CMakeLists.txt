add_library(sgl_render STATIC
    src/RenderQueue.cpp
    src/Shapes.cpp
)
target_include_directories(sgl_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(sgl_render PUBLIC cxx_std_23)
target_compile_options(sgl_render PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(sgl_render PUBLIC sgl_core)
