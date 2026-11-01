add_library(asteroids_sim STATIC
    src/AsteroidsGame.cpp
)
target_include_directories(asteroids_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(asteroids_sim PUBLIC cxx_std_23)
target_compile_options(asteroids_sim PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(asteroids_sim PUBLIC sgl_core)
