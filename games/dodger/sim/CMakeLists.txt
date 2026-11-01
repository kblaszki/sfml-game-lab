add_library(dodger_sim STATIC
    src/DodgerGame.cpp
)
target_include_directories(dodger_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(dodger_sim PUBLIC cxx_std_23)
target_compile_options(dodger_sim PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(dodger_sim PUBLIC sgl_core)
