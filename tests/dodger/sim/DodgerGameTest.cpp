#include <cmath>
#include <cstdint>
#include <dodger/sim/DodgerGame.hpp>
#include <gtest/gtest.h>
#include <variant>
#include <vector>

namespace
{

using sgl::Seconds;
using sgl::dodger::Died;
using sgl::dodger::DodgerEvent;
using sgl::dodger::DodgerGame;
using sgl::dodger::HazardCleared;
using sgl::dodger::HazardView;
using sgl::dodger::kPaddleY;
using sgl::dodger::kWorldW;
using sgl::dodger::PaddleView;
using sgl::dodger::SimInput;
using sgl::dodger::Tuning;

constexpr int kDeterminismSteps = 180;

void expectSameState(const DodgerGame& left, const DodgerGame& right)
{
    EXPECT_EQ(left.score(), right.score());
    EXPECT_EQ(left.over(), right.over());

    const PaddleView paddleLeft = left.paddle();
    const PaddleView paddleRight = right.paddle();
    EXPECT_EQ(paddleLeft.pos, paddleRight.pos);
    EXPECT_EQ(paddleLeft.size, paddleRight.size);

    const std::vector<HazardView>& hazardsLeft = left.hazards();
    const std::vector<HazardView>& hazardsRight = right.hazards();
    ASSERT_EQ(hazardsLeft.size(), hazardsRight.size());
    for(std::size_t i = 0; i < hazardsLeft.size(); ++i)
    {
        EXPECT_EQ(hazardsLeft[i].pos, hazardsRight[i].pos);
        EXPECT_EQ(hazardsLeft[i].size, hazardsRight[i].size);
    }
}

[[nodiscard]] bool sameEvent(const DodgerEvent& left, const DodgerEvent& right)
{
    return left.index() == right.index();
}

} // namespace

TEST(DodgerGameShould, leftAndRightMovePaddleX)
{
    Tuning tuning;
    tuning.spawnInterval = 10.f;
    DodgerGame game{1, tuning};

    const PaddleView start = game.paddle();
    EXPECT_FLOAT_EQ(start.pos.x, (kWorldW - tuning.paddleW) * 0.5f);
    EXPECT_FLOAT_EQ(start.pos.y, kPaddleY);
    EXPECT_FLOAT_EQ(start.size.x, tuning.paddleW);
    EXPECT_FLOAT_EQ(start.size.y, tuning.paddleH);
    EXPECT_EQ(game.score(), 0);
    EXPECT_FALSE(game.over());
    EXPECT_TRUE(game.hazards().empty());

    const Seconds dt{0.25f};
    const float distance = tuning.paddleSpeed * dt.count();

    SimInput right;
    right.right = true;
    game.step(right, dt);
    EXPECT_FLOAT_EQ(game.paddle().pos.x, start.pos.x + distance);
    EXPECT_FLOAT_EQ(game.paddle().pos.y, kPaddleY);

    DodgerGame leftGame{1, tuning};
    SimInput left;
    left.left = true;
    leftGame.step(left, dt);
    EXPECT_FLOAT_EQ(leftGame.paddle().pos.x, start.pos.x - distance);
    EXPECT_FLOAT_EQ(leftGame.paddle().pos.y, kPaddleY);

    DodgerGame both{1, tuning};
    SimInput cancel;
    cancel.left = true;
    cancel.right = true;
    both.step(cancel, dt);
    EXPECT_FLOAT_EQ(both.paddle().pos.x, start.pos.x);
    EXPECT_FLOAT_EQ(both.paddle().pos.y, kPaddleY);
}

TEST(DodgerGameShould, paddleClampsAtWorldEdges)
{
    Tuning tuning;
    tuning.paddleSpeed = 10000.f;
    tuning.spawnInterval = 10.f;
    const float maxX = kWorldW - tuning.paddleW;

    DodgerGame leftGame{1, tuning};
    SimInput left;
    left.left = true;
    leftGame.step(left, Seconds{1.f});
    EXPECT_FLOAT_EQ(leftGame.paddle().pos.x, 0.f);
    EXPECT_FLOAT_EQ(leftGame.paddle().pos.y, kPaddleY);

    DodgerGame rightGame{1, tuning};
    SimInput right;
    right.right = true;
    rightGame.step(right, Seconds{1.f});
    EXPECT_FLOAT_EQ(rightGame.paddle().pos.x, maxX);
    EXPECT_FLOAT_EQ(rightGame.paddle().pos.y, kPaddleY);
}

TEST(DodgerGameShould, spawnsHazardAfterSpawnInterval)
{
    Tuning tuning;
    tuning.hazardSpeed = 0.f;
    tuning.spawnInterval = 0.5f;
    DodgerGame early{7, tuning};
    early.step({}, Seconds{0.25f});
    EXPECT_TRUE(early.hazards().empty());

    DodgerGame once{7, tuning};
    once.step({}, Seconds{0.5f});
    ASSERT_EQ(once.hazards().size(), 1u);
    EXPECT_FLOAT_EQ(once.hazards()[0].pos.y, -tuning.hazardH);
    EXPECT_FLOAT_EQ(once.hazards()[0].size.x, tuning.hazardW);
    EXPECT_FLOAT_EQ(once.hazards()[0].size.y, tuning.hazardH);
    EXPECT_GE(once.hazards()[0].pos.x, 0.f);
    EXPECT_LT(once.hazards()[0].pos.x, kWorldW);
    EXPECT_FLOAT_EQ(std::fmod(once.hazards()[0].pos.x, tuning.hazardW), 0.f);

    DodgerGame twice{7, tuning};
    twice.step({}, Seconds{1.f});
    EXPECT_EQ(twice.hazards().size(), 2u);
}

TEST(DodgerGameShould, hazardPastBottomScoresAndClears)
{
    Tuning tuning;
    tuning.spawnInterval = 0.5f;
    tuning.hazardSpeed = 2000.f;
    DodgerGame game{3, tuning};

    const std::vector<DodgerEvent> events = game.step({}, Seconds{0.5f});
    EXPECT_EQ(game.score(), 1);
    EXPECT_TRUE(game.hazards().empty());
    EXPECT_FALSE(game.over());
    ASSERT_EQ(events.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<HazardCleared>(events[0]));
}

TEST(DodgerGameShould, overlapPaddleEndsGame)
{
    Tuning tuning;
    tuning.spawnInterval = 0.5f;
    tuning.hazardSpeed = 1360.f;
    tuning.hazardW = kWorldW;
    tuning.hazardH = 40.f;
    DodgerGame game{1, tuning};

    const std::vector<DodgerEvent> events = game.step({}, Seconds{0.5f});
    EXPECT_TRUE(game.over());
    EXPECT_EQ(game.score(), 0);
    ASSERT_EQ(game.hazards().size(), 1u);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<Died>(events[0]));

    const float frozenX = game.paddle().pos.x;
    SimInput right;
    right.right = true;
    const std::vector<DodgerEvent> after = game.step(right, Seconds{1.f});
    EXPECT_TRUE(after.empty());
    EXPECT_TRUE(game.over());
    EXPECT_EQ(game.score(), 0);
    EXPECT_FLOAT_EQ(game.paddle().pos.x, frozenX);
    EXPECT_EQ(game.hazards().size(), 1u);
}

TEST(DodgerGameShould, sameSeedMatchesAfterManySteps)
{
    DodgerGame left{42};
    DodgerGame right{42};
    for(int step = 0; step < kDeterminismSteps; ++step)
    {
        SimInput input;
        input.left = (step % 3) != 0;
        input.right = (step % 5) == 0;
        const std::vector<DodgerEvent> leftEvents = left.step(input, sgl::kTick);
        const std::vector<DodgerEvent> rightEvents = right.step(input, sgl::kTick);
        ASSERT_EQ(leftEvents.size(), rightEvents.size());
        for(std::size_t i = 0; i < leftEvents.size(); ++i)
        {
            EXPECT_TRUE(sameEvent(leftEvents[i], rightEvents[i]));
        }
    }
    expectSameState(left, right);
    EXPECT_FALSE(left.hazards().empty());
}
