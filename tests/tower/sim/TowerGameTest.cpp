#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <tower/sim/TowerGame.hpp>
#include <variant>
#include <vector>

namespace
{

using sgl::kTick;
using sgl::Seconds;
using sgl::tower::Died;
using sgl::tower::kViewH;
using sgl::tower::kWorldW;
using sgl::tower::Landed;
using sgl::tower::PlatformView;
using sgl::tower::PlayerView;
using sgl::tower::SimInput;
using sgl::tower::TowerEvent;
using sgl::tower::TowerGame;
using sgl::tower::Tuning;

constexpr int kDeterminismSteps = 180;

[[nodiscard]] SimInput inputFor(int step)
{
    SimInput input;
    input.left = step % 7 < 2;
    input.right = step % 5 < 2;
    input.jump = step % 11 == 0;
    return input;
}

void expectSameState(const TowerGame& left, const TowerGame& right)
{
    EXPECT_EQ(left.score(), right.score());
    EXPECT_EQ(left.over(), right.over());
    EXPECT_FLOAT_EQ(left.cameraBottom(), right.cameraBottom());

    const PlayerView playerLeft = left.player();
    const PlayerView playerRight = right.player();
    EXPECT_EQ(playerLeft.pos, playerRight.pos);
    EXPECT_EQ(playerLeft.vel, playerRight.vel);
    EXPECT_EQ(playerLeft.size, playerRight.size);
    EXPECT_EQ(playerLeft.grounded, playerRight.grounded);

    const std::vector<PlatformView>& platformsLeft = left.platforms();
    const std::vector<PlatformView>& platformsRight = right.platforms();
    ASSERT_EQ(platformsLeft.size(), platformsRight.size());
    for(std::size_t i = 0; i < platformsLeft.size(); ++i)
    {
        EXPECT_EQ(platformsLeft[i].pos, platformsRight[i].pos);
        EXPECT_EQ(platformsLeft[i].size, platformsRight[i].size);
    }
}

[[nodiscard]] bool sameEvent(const TowerEvent& left, const TowerEvent& right)
{
    return left.index() == right.index();
}

} // namespace

TEST(TowerGameShould, jumpOnlyWhenGrounded)
{
    Tuning tuning;
    tuning.iceFriction = 0.f;
    tuning.startPlatformW = kWorldW;
    const Seconds dt{1.f / 60.f};
    const float seconds = dt.count();

    TowerGame stayed{1, tuning};
    stayed.step({}, dt);
    EXPECT_TRUE(stayed.player().grounded);
    EXPECT_FLOAT_EQ(stayed.player().vel.y, 0.f);

    SimInput jump;
    jump.jump = true;
    TowerGame jumper{1, tuning};
    const std::vector<TowerEvent> takenOff = jumper.step(jump, dt);
    EXPECT_TRUE(takenOff.empty());
    EXPECT_FALSE(jumper.player().grounded);
    EXPECT_GT(jumper.player().pos.y, 0.f);
    EXPECT_FLOAT_EQ(jumper.player().vel.y, tuning.jumpSpeed - tuning.gravity * seconds);

    TowerGame held{1, tuning};
    held.step(jump, dt);
    const float before = held.player().vel.y;
    held.step(jump, dt);

    TowerGame released{1, tuning};
    released.step(jump, dt);
    released.step({}, dt);
    EXPECT_FALSE(held.player().grounded);
    EXPECT_FLOAT_EQ(held.player().vel.y, released.player().vel.y);
    EXPECT_FLOAT_EQ(held.player().vel.y, before - tuning.gravity * seconds);
    EXPECT_LT(held.player().vel.y, tuning.jumpSpeed);
}

TEST(TowerGameShould, iceFrictionSlowsVxOnGround)
{
    Tuning tuning;
    tuning.moveAccel = 100000.f;
    tuning.maxRunSpeed = 320.f;
    tuning.iceFriction = 1.8f;
    tuning.gravity = 0.f;
    const float dt = 0.05f;
    const float decay = std::exp(-tuning.iceFriction * dt);

    TowerGame game{1, tuning};
    ASSERT_TRUE(game.player().grounded);
    SimInput right;
    right.right = true;
    game.step(right, Seconds{dt});
    ASSERT_TRUE(game.player().grounded);
    const float afterAccel = game.player().vel.x;
    EXPECT_FLOAT_EQ(afterAccel, tuning.maxRunSpeed * decay);

    game.step({}, Seconds{dt});
    ASSERT_TRUE(game.player().grounded);
    EXPECT_LT(game.player().vel.x, afterAccel);
    EXPECT_FLOAT_EQ(game.player().vel.x, afterAccel * decay);
}

TEST(TowerGameShould, leftAndRightCancel)
{
    Tuning tuning;
    tuning.iceFriction = 0.f;
    tuning.gravity = 0.f;
    tuning.startPlatformW = kWorldW;
    const Seconds dt{0.05f};
    const float originX = (kWorldW - tuning.playerW) * 0.5f;

    TowerGame both{1, tuning};
    SimInput cancel;
    cancel.left = true;
    cancel.right = true;
    both.step(cancel, dt);
    EXPECT_TRUE(both.player().grounded);
    EXPECT_FLOAT_EQ(both.player().vel.x, 0.f);
    EXPECT_FLOAT_EQ(both.player().pos.x, originX);

    TowerGame left{1, tuning};
    SimInput leftInput;
    leftInput.left = true;
    left.step(leftInput, dt);

    TowerGame right{1, tuning};
    SimInput rightInput;
    rightInput.right = true;
    right.step(rightInput, dt);
    EXPECT_LT(left.player().vel.x, 0.f);
    EXPECT_GT(right.player().vel.x, 0.f);
    EXPECT_FLOAT_EQ(right.player().vel.x, -left.player().vel.x);
}

TEST(TowerGameShould, landingOnPlatformEmitsLanded)
{
    Tuning tuning;
    tuning.startPlatformW = kWorldW;
    tuning.iceFriction = 0.f;
    TowerGame game{1, tuning};
    ASSERT_TRUE(game.player().grounded);

    SimInput jump;
    jump.jump = true;
    game.step(jump, kTick);
    ASSERT_FALSE(game.player().grounded);

    int landed = 0;
    for(int i = 0; i < 180 && !game.player().grounded && !game.over(); ++i)
    {
        const std::vector<TowerEvent> events = game.step({}, kTick);
        for(const TowerEvent& event: events)
        {
            if(std::holds_alternative<Landed>(event))
            {
                ++landed;
            }
        }
    }

    EXPECT_FALSE(game.over());
    EXPECT_TRUE(game.player().grounded);
    EXPECT_EQ(landed, 1);
    EXPECT_FLOAT_EQ(game.player().vel.y, 0.f);
}

TEST(TowerGameShould, cameraOnlyRises)
{
    Tuning tuning;
    tuning.startPlatformW = kWorldW;
    tuning.iceFriction = 0.f;
    TowerGame game{1, tuning};
    const float initial = game.cameraBottom();
    EXPECT_FLOAT_EQ(initial, game.player().pos.y - kViewH * 0.65f);

    float previous = initial;
    bool rose = false;
    for(int i = 0; i < 180 && !game.over(); ++i)
    {
        SimInput input;
        input.jump = i == 0;
        game.step(input, kTick);
        const float camera = game.cameraBottom();
        EXPECT_GE(camera, previous);
        if(camera > previous)
        {
            rose = true;
        }
        previous = camera;
    }

    EXPECT_TRUE(rose);
    EXPECT_FALSE(game.over());
    const float peakCamera = game.cameraBottom();
    for(int i = 0; i < 60 && !game.over(); ++i)
    {
        game.step({}, kTick);
        EXPECT_GE(game.cameraBottom(), peakCamera);
    }
    EXPECT_FLOAT_EQ(game.cameraBottom(), peakCamera);
}

TEST(TowerGameShould, fallBelowCameraDies)
{
    Tuning tuning;
    tuning.startPlatformW = 40.f;
    tuning.playerW = 40.f;
    tuning.moveAccel = 8000.f;
    tuning.maxRunSpeed = 600.f;
    tuning.iceFriction = 0.f;
    tuning.gravity = 4000.f;
    TowerGame game{1, tuning};
    const float camera = game.cameraBottom();
    EXPECT_FALSE(game.over());

    SimInput right;
    right.right = true;
    bool died = false;
    for(int i = 0; i < 600 && !game.over(); ++i)
    {
        const std::vector<TowerEvent> events = game.step(right, kTick);
        for(const TowerEvent& event: events)
        {
            if(std::holds_alternative<Died>(event))
            {
                died = true;
            }
        }
    }

    EXPECT_TRUE(died);
    EXPECT_TRUE(game.over());
    EXPECT_FLOAT_EQ(game.cameraBottom(), camera);
    EXPECT_LT(game.player().pos.y + game.player().size.y, game.cameraBottom() - tuning.deathMargin);

    const int score = game.score();
    const PlayerView frozen = game.player();
    const std::size_t platformCount = game.platforms().size();
    const std::vector<TowerEvent> after = game.step(right, Seconds{1.f});
    EXPECT_TRUE(after.empty());
    EXPECT_TRUE(game.over());
    EXPECT_EQ(game.score(), score);
    EXPECT_FLOAT_EQ(game.cameraBottom(), camera);
    EXPECT_EQ(game.player().pos, frozen.pos);
    EXPECT_EQ(game.player().vel, frozen.vel);
    EXPECT_EQ(game.player().grounded, frozen.grounded);
    EXPECT_EQ(game.platforms().size(), platformCount);
}

TEST(TowerGameShould, sameSeedMatchesAfterManySteps)
{
    TowerGame left{42, {}};
    TowerGame right{42, {}};
    std::vector<TowerEvent> leftEvents;
    std::vector<TowerEvent> rightEvents;
    for(int step = 0; step < kDeterminismSteps; ++step)
    {
        const SimInput input = inputFor(step);
        const std::vector<TowerEvent> leftStep = left.step(input, kTick);
        const std::vector<TowerEvent> rightStep = right.step(input, kTick);
        ASSERT_EQ(leftStep.size(), rightStep.size());
        for(std::size_t i = 0; i < leftStep.size(); ++i)
        {
            EXPECT_TRUE(sameEvent(leftStep[i], rightStep[i]));
        }
        leftEvents.insert(leftEvents.end(), leftStep.begin(), leftStep.end());
        rightEvents.insert(rightEvents.end(), rightStep.begin(), rightStep.end());
    }

    expectSameState(left, right);
    ASSERT_EQ(leftEvents.size(), rightEvents.size());
    for(std::size_t i = 0; i < leftEvents.size(); ++i)
    {
        EXPECT_TRUE(sameEvent(leftEvents[i], rightEvents[i]));
    }
}

TEST(TowerGameShould, scoreTracksPeakHeight)
{
    Tuning tuning;
    tuning.gravity = 0.f;
    tuning.jumpSpeed = 100.5f;
    tuning.iceFriction = 0.f;
    TowerGame hover{1, tuning};
    EXPECT_EQ(hover.score(), 0);

    SimInput jump;
    jump.jump = true;
    hover.step(jump, Seconds{1.f});
    EXPECT_FLOAT_EQ(hover.player().pos.y, 100.5f);
    EXPECT_EQ(hover.score(), 100);

    hover.step({}, Seconds{1.f});
    EXPECT_FLOAT_EQ(hover.player().pos.y, 201.f);
    EXPECT_EQ(hover.score(), 201);

    Tuning fallingTuning;
    fallingTuning.startPlatformW = kWorldW;
    fallingTuning.iceFriction = 0.f;
    TowerGame falling{3, fallingTuning};
    float peakFeet = falling.player().pos.y;
    EXPECT_EQ(falling.score(), static_cast<int>(std::floor(peakFeet)));

    for(int i = 0; i < 240 && !falling.over(); ++i)
    {
        SimInput input;
        input.jump = i == 0;
        falling.step(input, kTick);
        peakFeet = std::max(peakFeet, falling.player().pos.y);
        EXPECT_EQ(falling.score(), static_cast<int>(std::floor(peakFeet)));
    }
    EXPECT_GT(falling.score(), 0);
    EXPECT_FALSE(falling.over());
}
