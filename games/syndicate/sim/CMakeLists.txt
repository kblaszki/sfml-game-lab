add_library(syndicate_sim STATIC
    src/BoardGraph.cpp
    src/BoardLayouts.cpp
    src/Pathing.cpp
)
target_include_directories(syndicate_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(syndicate_sim PUBLIC cxx_std_23)
target_compile_options(syndicate_sim PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(syndicate_sim PUBLIC sgl_core)

foreach(part economy cards auction match bot)
    include(${CMAKE_CURRENT_SOURCE_DIR}/parts/${part}.cmake OPTIONAL)
endforeach()
