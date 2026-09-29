add_unit_test(syndicate_money_test
    SOURCES
        MoneyTest.cpp
    DEPENDENCIES
        syndicate_sim
)

add_unit_test(syndicate_board_graph_test
    SOURCES
        BoardGraphTest.cpp
    DEPENDENCIES
        syndicate_sim
)

add_unit_test(syndicate_board_layouts_test
    SOURCES
        BoardLayoutsTest.cpp
    DEPENDENCIES
        syndicate_sim
)

add_unit_test(syndicate_pathing_test
    SOURCES
        PathingTest.cpp
    DEPENDENCIES
        syndicate_sim
)

foreach(part economy cards auction match bot)
    include(${CMAKE_CURRENT_SOURCE_DIR}/parts/${part}.cmake OPTIONAL)
endforeach()
