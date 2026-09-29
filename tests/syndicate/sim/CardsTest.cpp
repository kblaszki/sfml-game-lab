#include <array>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <map>
#include <sgl/core/Random.hpp>
#include <syndicate/sim/Cards.hpp>
#include <variant>
#include <vector>

using namespace sgl::syndicate;

namespace
{

constexpr std::array<CardKind, 8> kKinds = {
    CardKind::HostileTakeover,
    CardKind::Audit,
    CardKind::MarketCrash,
    CardKind::ShortSqueeze,
    CardKind::Lobbying,
    CardKind::Embargo,
    CardKind::Subsidy,
    CardKind::Insider,
};

[[nodiscard]] std::map<CardKind, int> multisetOf(const std::vector<CardKind>& cards)
{
    std::map<CardKind, int> counts;
    for(const CardKind k: cards)
    {
        ++counts[k];
    }
    return counts;
}

[[nodiscard]] std::map<CardKind, int> fullComposition()
{
    std::map<CardKind, int> counts;
    for(const CardKind k: kKinds)
    {
        counts[k] = 2;
    }
    return counts;
}

} // namespace

TEST(DeckShould, SameSeedYieldsSameDrawOrder)
{
    constexpr std::uint64_t seed = 42;
    sgl::Pcg32 rngA{seed};
    sgl::Pcg32 rngB{seed};
    Deck deckA;
    Deck deckB;
    deckA.fillAndShuffle(rngA);
    deckB.fillAndShuffle(rngB);

    std::vector<CardKind> orderA;
    std::vector<CardKind> orderB;
    orderA.reserve(16);
    orderB.reserve(16);
    for(int i = 0; i < 16; ++i)
    {
        const auto a = deckA.draw(rngA);
        const auto b = deckB.draw(rngB);
        ASSERT_TRUE(a.has_value());
        ASSERT_TRUE(b.has_value());
        orderA.push_back(*a);
        orderB.push_back(*b);
    }
    EXPECT_EQ(orderA, orderB);
    EXPECT_EQ(deckA.drawCount(), 0u);
    EXPECT_EQ(deckB.drawCount(), 0u);
}

TEST(DeckShould, ReshuffleConservesFullComposition)
{
    sgl::Pcg32 rng{7};
    Deck deck;
    deck.fillAndShuffle(rng);

    std::vector<CardKind> first;
    first.reserve(16);
    for(int i = 0; i < 16; ++i)
    {
        const auto card = deck.draw(rng);
        ASSERT_TRUE(card.has_value());
        first.push_back(*card);
    }
    EXPECT_EQ(multisetOf(first), fullComposition());

    for(const CardKind k: first)
    {
        deck.discard(k);
    }
    EXPECT_EQ(deck.drawCount(), 0u);
    EXPECT_EQ(deck.discardCount(), 16u);

    std::vector<CardKind> second;
    second.reserve(16);
    for(int i = 0; i < 16; ++i)
    {
        const auto card = deck.draw(rng);
        ASSERT_TRUE(card.has_value());
        second.push_back(*card);
    }
    EXPECT_EQ(multisetOf(second), fullComposition());
    EXPECT_EQ(deck.drawCount(), 0u);
    EXPECT_EQ(deck.discardCount(), 0u);
}

TEST(HandShould, SixthAddReturnsHandFull)
{
    Hand hand;
    for(std::size_t i = 0; i < Hand::kMax; ++i)
    {
        ASSERT_TRUE(hand.add(CardKind::Subsidy).has_value());
    }
    EXPECT_EQ(hand.cards().size(), Hand::kMax);
    const auto sixth = hand.add(CardKind::Insider);
    ASSERT_FALSE(sixth.has_value());
    EXPECT_EQ(sixth.error(), CardError::HandFull);
    EXPECT_EQ(hand.cards().size(), Hand::kMax);
}

TEST(CardsShould, ValidateTargetRejectsWrongAlternative)
{
    struct Case
    {
        CardKind kind;
        CardTarget wrong;
    };

    const Case cases[] = {
        {CardKind::HostileTakeover, NoTarget{}},
        {CardKind::HostileTakeover, SectorTarget{Sector::AI}},
        {CardKind::HostileTakeover, PlayerTarget{PlayerId{0}}},
        {CardKind::HostileTakeover, TileTarget{NodeId{0}}},
        {CardKind::Audit, NoTarget{}},
        {CardKind::Audit, SectorTarget{Sector::AI}},
        {CardKind::Audit, TileTarget{NodeId{0}}},
        {CardKind::Audit, TakeoverTarget{PlayerId{0}, NodeId{0}}},
        {CardKind::MarketCrash, NoTarget{}},
        {CardKind::MarketCrash, PlayerTarget{PlayerId{0}}},
        {CardKind::MarketCrash, TileTarget{NodeId{0}}},
        {CardKind::MarketCrash, TakeoverTarget{PlayerId{0}, NodeId{0}}},
        {CardKind::ShortSqueeze, NoTarget{}},
        {CardKind::ShortSqueeze, PlayerTarget{PlayerId{0}}},
        {CardKind::ShortSqueeze, TileTarget{NodeId{0}}},
        {CardKind::ShortSqueeze, TakeoverTarget{PlayerId{0}, NodeId{0}}},
        {CardKind::Lobbying, NoTarget{}},
        {CardKind::Lobbying, SectorTarget{Sector::AI}},
        {CardKind::Lobbying, PlayerTarget{PlayerId{0}}},
        {CardKind::Lobbying, TakeoverTarget{PlayerId{0}, NodeId{0}}},
        {CardKind::Embargo, NoTarget{}},
        {CardKind::Embargo, SectorTarget{Sector::AI}},
        {CardKind::Embargo, PlayerTarget{PlayerId{0}}},
        {CardKind::Embargo, TakeoverTarget{PlayerId{0}, NodeId{0}}},
        {CardKind::Subsidy, SectorTarget{Sector::AI}},
        {CardKind::Subsidy, PlayerTarget{PlayerId{0}}},
        {CardKind::Subsidy, TileTarget{NodeId{0}}},
        {CardKind::Subsidy, TakeoverTarget{PlayerId{0}, NodeId{0}}},
        {CardKind::Insider, SectorTarget{Sector::AI}},
        {CardKind::Insider, PlayerTarget{PlayerId{0}}},
        {CardKind::Insider, TileTarget{NodeId{0}}},
        {CardKind::Insider, TakeoverTarget{PlayerId{0}, NodeId{0}}},
    };

    for(const Case& c: cases)
    {
        const auto result = validateTarget(c.kind, c.wrong);
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), CardError::BadTarget);
    }
}

TEST(CardsShould, MarketCrashDeltaIsNeg30)
{
    const auto effects = describeEffect(CardKind::MarketCrash, SectorTarget{Sector::Fintech});
    ASSERT_TRUE(effects.has_value());
    ASSERT_EQ(effects->size(), 1u);
    const auto* delta = std::get_if<MarketDelta>(&(*effects)[0]);
    ASSERT_NE(delta, nullptr);
    EXPECT_EQ(delta->sector, Sector::Fintech);
    EXPECT_EQ(delta->delta, -30);
}
