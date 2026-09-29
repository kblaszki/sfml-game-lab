add_library(sgl_ui STATIC
    src/MenuModel.cpp
    src/RenderMenu.cpp
    src/PauseOverlay.cpp
    src/TextLayout.cpp
    src/TooltipSystem.cpp
    src/HoverAnim.cpp
)
target_include_directories(sgl_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(sgl_ui PUBLIC cxx_std_23)
target_compile_options(sgl_ui PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(sgl_ui PUBLIC sgl_input sgl_render sgl_scene sgl_anim)
