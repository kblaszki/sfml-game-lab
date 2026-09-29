add_library(sgl_anim STATIC
    src/Spline.cpp
)
target_include_directories(sgl_anim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(sgl_anim PUBLIC cxx_std_23)
target_compile_options(sgl_anim PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(sgl_anim PUBLIC sgl_core)
