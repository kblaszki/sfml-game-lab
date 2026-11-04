add_library(tower_sim STATIC
    src/TowerGame.cpp
)
target_include_directories(tower_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(tower_sim PUBLIC cxx_std_23)
target_compile_options(tower_sim PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(tower_sim PUBLIC sgl_core)
