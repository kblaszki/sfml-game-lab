#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <syndicate/sim/Trade.hpp>

using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayerId;
using sgl::syndicate::ShareLot;
using sgl::syndicate::TradeError;
using sgl::syndicate::TradeOffer;
using sgl::syndicate::kMaxPlayers;
using sgl::syndicate::validateTrade;

namespace
{

PlayerId P(std::uint8_t i)
{
    return PlayerId{i};
}

} // namespace

TEST(TradeTest, SharesPlayerDoesNotHaveIsInsufficientShares)
{
    std::array<Money, kMaxPlayers> cash{Money{100}, Money{100}, Money{}, Money{}};
    TradeOffer offer{
        .from = P(0),
        .to = P(1),
        .cashFrom = Money{0},
        .cashTo = Money{0},
        .sharesFrom = {ShareLot{NodeId{3}, 2}},
        .sharesTo = {},
    };

    auto sharesOf = [](PlayerId who, NodeId tile) -> std::uint8_t {
        if(who == P(0) && tile == NodeId{3})
        {
            return 1;
        }
        return 0;
    };

    auto result = validateTrade(offer, cash, sharesOf);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), TradeError::InsufficientShares);
}

TEST(TradeTest, HappyPathCashAndShares)
{
    std::array<Money, kMaxPlayers> cash{Money{80}, Money{50}, Money{}, Money{}};
    TradeOffer offer{
        .from = P(0),
        .to = P(1),
        .cashFrom = Money{20},
        .cashTo = Money{10},
        .sharesFrom = {ShareLot{NodeId{1}, 1}},
        .sharesTo = {ShareLot{NodeId{2}, 2}},
    };

    auto sharesOf = [](PlayerId who, NodeId tile) -> std::uint8_t {
        if(who == P(0) && tile == NodeId{1})
        {
            return 3;
        }
        if(who == P(1) && tile == NodeId{2})
        {
            return 2;
        }
        return 0;
    };

    auto result = validateTrade(offer, cash, sharesOf);
    EXPECT_TRUE(result.has_value());
}

TEST(TradeTest, SamePlayerRejected)
{
    std::array<Money, kMaxPlayers> cash{};
    TradeOffer offer{
        .from = P(0),
        .to = P(0),
        .cashFrom = Money{1},
        .cashTo = Money{0},
        .sharesFrom = {},
        .sharesTo = {},
    };
    auto result = validateTrade(offer, cash, [](PlayerId, NodeId) -> std::uint8_t { return 0; });
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), TradeError::SamePlayer);
}

TEST(TradeTest, EmptyOfferRejected)
{
    std::array<Money, kMaxPlayers> cash{Money{10}, Money{10}, Money{}, Money{}};
    TradeOffer offer{
        .from = P(0),
        .to = P(1),
        .cashFrom = Money{0},
        .cashTo = Money{0},
        .sharesFrom = {},
        .sharesTo = {},
    };
    auto result = validateTrade(offer, cash, [](PlayerId, NodeId) -> std::uint8_t { return 0; });
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), TradeError::Empty);
}

TEST(TradeTest, InsufficientCashRejected)
{
    std::array<Money, kMaxPlayers> cash{Money{5}, Money{100}, Money{}, Money{}};
    TradeOffer offer{
        .from = P(0),
        .to = P(1),
        .cashFrom = Money{10},
        .cashTo = Money{0},
        .sharesFrom = {},
        .sharesTo = {},
    };
    auto result = validateTrade(offer, cash, [](PlayerId, NodeId) -> std::uint8_t { return 0; });
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), TradeError::InsufficientCash);
}
