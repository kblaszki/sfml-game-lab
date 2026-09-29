add_library(sgl_resources STATIC
    src/Atlas.cpp
)
target_include_directories(sgl_resources PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(sgl_resources PUBLIC cxx_std_23)
target_compile_options(sgl_resources PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(sgl_resources PUBLIC sgl_core)
