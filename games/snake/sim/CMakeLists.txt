add_library(snake_sim STATIC
    src/SnakeGame.cpp
)
target_include_directories(snake_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(snake_sim PUBLIC cxx_std_23)
target_compile_options(snake_sim PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(snake_sim PUBLIC sgl_core)
