#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <syndicate/sim/Auction.hpp>
#include <variant>

using sgl::syndicate::AuctionContinue;
using sgl::syndicate::AuctionError;
using sgl::syndicate::AuctionOutcome;
using sgl::syndicate::AuctionState;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayerId;
using sgl::syndicate::Sold;
using sgl::syndicate::Unsold;
using sgl::syndicate::bid;
using sgl::syndicate::kMaxPlayers;
using sgl::syndicate::passBid;
using sgl::syndicate::startAuction;

namespace
{

PlayerId P(std::uint8_t i)
{
    return PlayerId{i};
}

std::array<bool, kMaxPlayers> allEligible(int seats)
{
    std::array<bool, kMaxPlayers> e{};
    for(int i = 0; i < seats; ++i)
    {
        e[static_cast<std::size_t>(i)] = true;
    }
    return e;
}

} // namespace

TEST(AuctionTest, BidBelowPlusTenRejected)
{
    auto state = startAuction(NodeId{1}, 3, Money{50}, 3, P(0), allEligible(3));
    auto first = bid(state, P(0), Money{50}, Money{200});
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(std::holds_alternative<AuctionContinue>(*first));

    auto tooLow = bid(state, P(1), Money{59}, Money{200});
    ASSERT_FALSE(tooLow.has_value());
    EXPECT_EQ(tooLow.error(), AuctionError::BidTooLow);

    auto ok = bid(state, P(1), Money{60}, Money{200});
    ASSERT_TRUE(ok.has_value());
    EXPECT_TRUE(std::holds_alternative<AuctionContinue>(*ok));
    EXPECT_EQ(state.highBid, (Money{60}));
    EXPECT_EQ(state.leader, P(1));
}

TEST(AuctionTest, BidWhenNotToActIsNotYourTurn)
{
    auto state = startAuction(NodeId{1}, 3, Money{50}, 3, P(0), allEligible(3));
    auto result = bid(state, P(1), Money{50}, Money{200});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), AuctionError::NotYourTurn);
}

TEST(AuctionTest, IneligibleDebtorNeverBecomesToAct)
{
    std::array<bool, kMaxPlayers> eligible = {true, false, true, false};
    auto state = startAuction(NodeId{2}, 3, Money{40}, 3, P(1), eligible);
    EXPECT_FALSE(state.active[1]);
    EXPECT_EQ(state.toAct, P(2));

    auto result = bid(state, P(1), Money{40}, Money{0});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), AuctionError::NotYourTurn);
}

TEST(AuctionTest, EveryonePassesWithNoBidsIsUnsold)
{
    auto state = startAuction(NodeId{1}, 3, Money{50}, 3, P(0), allEligible(3));

    auto a = passBid(state, P(0));
    ASSERT_TRUE(a.has_value());
    EXPECT_TRUE(std::holds_alternative<AuctionContinue>(*a));

    auto b = passBid(state, P(1));
    ASSERT_TRUE(b.has_value());
    EXPECT_TRUE(std::holds_alternative<AuctionContinue>(*b));

    auto c = passBid(state, P(2));
    ASSERT_TRUE(c.has_value());
    ASSERT_TRUE(std::holds_alternative<Unsold>(*c));
}

TEST(AuctionTest, SoleRemainingBidderWins)
{
    auto state = startAuction(NodeId{1}, 3, Money{50}, 3, P(0), allEligible(3));

    ASSERT_TRUE(bid(state, P(0), Money{50}, Money{200}));
    ASSERT_TRUE(passBid(state, P(1)));
    auto lastPass = passBid(state, P(2));
    ASSERT_TRUE(lastPass.has_value());
    ASSERT_TRUE(std::holds_alternative<Sold>(*lastPass));
    const auto& sold = std::get<Sold>(*lastPass);
    EXPECT_EQ(sold.winner, P(0));
    EXPECT_EQ(sold.price, (Money{50}));
}

TEST(AuctionTest, HappyPathThreePlayerBidSequence)
{
    auto state = startAuction(NodeId{5}, 3, Money{100}, 3, P(0), allEligible(3));
    EXPECT_EQ(state.toAct, P(0));
    EXPECT_EQ(state.highBid, (Money{100}));
    EXPECT_FALSE(state.leader.has_value());

    auto b0 = bid(state, P(0), Money{100}, Money{500});
    ASSERT_TRUE(b0.has_value());
    EXPECT_TRUE(std::holds_alternative<AuctionContinue>(*b0));
    EXPECT_EQ(state.leader, P(0));
    EXPECT_EQ(state.toAct, P(1));

    auto b1 = bid(state, P(1), Money{110}, Money{500});
    ASSERT_TRUE(b1.has_value());
    EXPECT_TRUE(std::holds_alternative<AuctionContinue>(*b1));
    EXPECT_EQ(state.leader, P(1));
    EXPECT_EQ(state.highBid, (Money{110}));
    EXPECT_EQ(state.toAct, P(2));

    auto b2 = bid(state, P(2), Money{120}, Money{500});
    ASSERT_TRUE(b2.has_value());
    EXPECT_TRUE(std::holds_alternative<AuctionContinue>(*b2));
    EXPECT_EQ(state.leader, P(2));
    EXPECT_EQ(state.toAct, P(0));

    auto p0 = passBid(state, P(0));
    ASSERT_TRUE(p0.has_value());
    EXPECT_TRUE(std::holds_alternative<AuctionContinue>(*p0));
    EXPECT_EQ(state.toAct, P(1));

    auto p1 = passBid(state, P(1));
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(std::holds_alternative<Sold>(*p1));
    const auto& sold = std::get<Sold>(*p1);
    EXPECT_EQ(sold.winner, P(2));
    EXPECT_EQ(sold.price, (Money{120}));
}
