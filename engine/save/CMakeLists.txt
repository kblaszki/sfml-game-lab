add_library(sgl_save STATIC
    src/HighScoreTable.cpp
    src/SaveError.cpp
    src/SaveFile.cpp
    src/SaveFormat.cpp
    src/ScoreStore.cpp
    src/Settings.cpp
)
target_include_directories(sgl_save PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(sgl_save PUBLIC cxx_std_23)
target_compile_options(sgl_save PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(sgl_save PUBLIC sgl_core)
