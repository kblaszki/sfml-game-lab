add_unit_test(syndicate_market_test SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/MarketTest.cpp DEPENDENCIES syndicate_sim)
add_unit_test(syndicate_economy_test SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/EconomyTest.cpp DEPENDENCIES syndicate_sim)
