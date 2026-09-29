add_unit_test(syndicate_auction_test SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/AuctionTest.cpp DEPENDENCIES syndicate_sim)
add_unit_test(syndicate_trade_test SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/TradeTest.cpp DEPENDENCIES syndicate_sim)
