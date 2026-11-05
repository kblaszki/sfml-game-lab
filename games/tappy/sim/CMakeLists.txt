add_library(tappy_sim STATIC
    src/TappyGame.cpp
)
target_include_directories(tappy_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(tappy_sim PUBLIC cxx_std_23)
target_compile_options(tappy_sim PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(tappy_sim PUBLIC sgl_core)
