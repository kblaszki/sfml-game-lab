#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <tappy/sim/TappyGame.hpp>
#include <variant>
#include <vector>

namespace
{

using sgl::kTick;
using sgl::Seconds;
using sgl::tappy::Died;
using sgl::tappy::Flapped;
using sgl::tappy::GateCleared;
using sgl::tappy::GateView;
using sgl::tappy::kWorldH;
using sgl::tappy::PlayerView;
using sgl::tappy::SimInput;
using sgl::tappy::TappyEvent;
using sgl::tappy::TappyGame;
using sgl::tappy::Tuning;

void expectSameState(const TappyGame& left, const TappyGame& right)
{
    EXPECT_EQ(left.score(), right.score());
    EXPECT_EQ(left.over(), right.over());
    EXPECT_FLOAT_EQ(left.cameraLeft(), right.cameraLeft());
    EXPECT_FLOAT_EQ(left.distance(), right.distance());
    EXPECT_FLOAT_EQ(left.scrollSpeed(), right.scrollSpeed());

    const PlayerView pl = left.player();
    const PlayerView pr = right.player();
    EXPECT_EQ(pl.pos, pr.pos);
    EXPECT_EQ(pl.vel, pr.vel);
    EXPECT_EQ(pl.size, pr.size);

    ASSERT_EQ(left.gates().size(), right.gates().size());
    for(std::size_t i = 0; i < left.gates().size(); ++i)
    {
        const GateView& a = left.gates()[i];
        const GateView& b = right.gates()[i];
        EXPECT_EQ(a.topPos, b.topPos);
        EXPECT_EQ(a.topSize, b.topSize);
        EXPECT_EQ(a.bottomPos, b.bottomPos);
        EXPECT_EQ(a.bottomSize, b.bottomSize);
        EXPECT_EQ(a.cleared, b.cleared);
    }
}

} // namespace

TEST(TappyGameShould, flapOverwritesVyAndEmits)
{
    Tuning tuning;
    tuning.gravity = 0.f;
    TappyGame game{1, tuning};

    SimInput flap;
    flap.flap = true;
    const std::vector<TappyEvent> events = game.step(flap, kTick);
    EXPECT_TRUE(std::any_of(events.begin(), events.end(), [](const TappyEvent& e) {
        return std::holds_alternative<Flapped>(e);
    }));
    EXPECT_FLOAT_EQ(game.player().vel.y, tuning.flapSpeed);

    game.step({}, kTick);
    EXPECT_FLOAT_EQ(game.player().vel.y, tuning.flapSpeed);

    flap.flap = true;
    game.step(flap, kTick);
    EXPECT_FLOAT_EQ(game.player().vel.y, tuning.flapSpeed);
}

TEST(TappyGameShould, gravityLowersVy)
{
    Tuning tuning;
    tuning.flapSpeed = 100.f;
    tuning.gravity = 600.f;
    TappyGame game{1, tuning};

    SimInput flap;
    flap.flap = true;
    game.step(flap, Seconds{1.f / 60.f});
    const float afterFlap = game.player().vel.y;
    game.step({}, Seconds{1.f / 60.f});
    EXPECT_LT(game.player().vel.y, afterFlap);
}

TEST(TappyGameShould, sameSeedMatchesAfterManySteps)
{
    Tuning tuning;
    TappyGame left{42, tuning};
    TappyGame right{42, tuning};
    for(int i = 0; i < 180; ++i)
    {
        SimInput input;
        input.flap = i % 17 == 0;
        const auto el = left.step(input, kTick);
        const auto er = right.step(input, kTick);
        ASSERT_EQ(el.size(), er.size());
        for(std::size_t j = 0; j < el.size(); ++j)
        {
            EXPECT_EQ(el[j].index(), er[j].index());
            if(const auto* a = std::get_if<GateCleared>(&el[j]))
            {
                const auto* b = std::get_if<GateCleared>(&er[j]);
                ASSERT_NE(b, nullptr);
                EXPECT_EQ(a->gateIndex, b->gateIndex);
            }
        }
        expectSameState(left, right);
        if(left.over())
        {
            break;
        }
    }
}

TEST(TappyGameShould, clearingGateIncreasesScore)
{
    Tuning tuning;
    tuning.gravity = 0.f;
    tuning.flapSpeed = 0.f;
    tuning.scrollBase = 400.f;
    tuning.scrollAccel = 0.f;
    tuning.gateSpacing = 200.f;
    tuning.gapStart = 400.f;
    tuning.gapMin = 400.f;
    tuning.gapShrink = 0.f;
    tuning.playerSpawnY = 360.f - 18.f;
    tuning.playerH = 36.f;
    TappyGame game{1, tuning};
    EXPECT_EQ(game.score(), 0);
    ASSERT_FALSE(game.gates().empty());

    bool cleared = false;
    for(int i = 0; i < 300 && !game.over(); ++i)
    {
        const std::vector<TappyEvent> events = game.step({}, kTick);
        for(const TappyEvent& event: events)
        {
            if(std::holds_alternative<GateCleared>(event))
            {
                cleared = true;
            }
        }
        if(cleared)
        {
            break;
        }
    }
    EXPECT_TRUE(cleared);
    EXPECT_GE(game.score(), 1);
}

TEST(TappyGameShould, hittingGateDies)
{
    Tuning tuning;
    tuning.gravity = 0.f;
    tuning.scrollBase = 500.f;
    tuning.scrollAccel = 0.f;
    tuning.gateSpacing = 120.f;
    tuning.gapStart = 40.f;
    tuning.gapMin = 40.f;
    tuning.gapShrink = 0.f;
    tuning.gapCenterMargin = 0.f;
    tuning.playerSpawnY = 0.f;
    TappyGame game{1, tuning};

    bool died = false;
    for(int i = 0; i < 240 && !died; ++i)
    {
        for(const TappyEvent& event: game.step({}, kTick))
        {
            died = died || std::holds_alternative<Died>(event);
        }
    }
    EXPECT_TRUE(died);
    EXPECT_TRUE(game.over());
}

TEST(TappyGameShould, floorOrCeilingKills)
{
    Tuning tuning;
    tuning.gravity = 5000.f;
    tuning.scrollBase = 0.f;
    tuning.scrollAccel = 0.f;
    tuning.gateSpacing = 10000.f;
    TappyGame game{1, tuning};

    bool died = false;
    for(int i = 0; i < 120 && !died; ++i)
    {
        for(const TappyEvent& event: game.step({}, kTick))
        {
            died = died || std::holds_alternative<Died>(event);
        }
    }
    EXPECT_TRUE(died);
    EXPECT_TRUE(game.over());
    EXPECT_LT(game.player().pos.y, 1.f);
}

TEST(TappyGameShould, scrollSpeedRampsAndCaps)
{
    Tuning tuning;
    tuning.gravity = 0.f;
    tuning.scrollBase = 220.f;
    tuning.scrollAccel = 0.1f;
    tuning.scrollCap = 300.f;
    tuning.gateSpacing = 10000.f;
    tuning.playerSpawnX = 0.f;
    TappyGame game{1, tuning};
    EXPECT_FLOAT_EQ(game.scrollSpeed(), tuning.scrollBase);

    for(int i = 0; i < 600; ++i)
    {
        game.step({}, kTick);
    }
    EXPECT_NEAR(game.scrollSpeed(), tuning.scrollCap, 0.01f);
    EXPECT_LE(game.scrollSpeed(), tuning.scrollCap);
}
