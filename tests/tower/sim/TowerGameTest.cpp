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
using sgl::tower::BonusKind;
using sgl::tower::BonusPicked;
using sgl::tower::BonusView;
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
using sgl::tower::WallBounced;

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

    const std::vector<BonusView>& bonusesLeft = left.bonuses();
    const std::vector<BonusView>& bonusesRight = right.bonuses();
    ASSERT_EQ(bonusesLeft.size(), bonusesRight.size());
    for(std::size_t i = 0; i < bonusesLeft.size(); ++i)
    {
        EXPECT_EQ(bonusesLeft[i].pos, bonusesRight[i].pos);
        EXPECT_EQ(bonusesLeft[i].size, bonusesRight[i].size);
        EXPECT_EQ(bonusesLeft[i].kind, bonusesRight[i].kind);
    }
    EXPECT_FLOAT_EQ(left.runSpeedCap(), right.runSpeedCap());
    EXPECT_FLOAT_EQ(left.jumpSpeedEffective(), right.jumpSpeedEffective());
}

[[nodiscard]] bool sameEvent(const TowerEvent& left, const TowerEvent& right)
{
    if(left.index() != right.index())
    {
        return false;
    }
    if(const auto* a = std::get_if<BonusPicked>(&left))
    {
        const auto* b = std::get_if<BonusPicked>(&right);
        return b != nullptr && a->kind == b->kind && a->points == b->points;
    }
    return true;
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

TEST(TowerGameShould, heldJumpAutojumpsAfterLand)
{
    Tuning tuning;
    tuning.startPlatformW = kWorldW;
    tuning.iceFriction = 0.f;
    TowerGame game{1, tuning};

    SimInput jump;
    jump.jump = true;
    game.step(jump, kTick);
    ASSERT_FALSE(game.player().grounded);

    bool landed = false;
    for(int i = 0; i < 240 && !game.over(); ++i)
    {
        const std::vector<TowerEvent> events = game.step(jump, kTick);
        for(const TowerEvent& event: events)
        {
            if(std::holds_alternative<Landed>(event))
            {
                landed = true;
            }
        }
        if(landed)
        {
            break;
        }
    }
    ASSERT_TRUE(landed);

    // Still holding jump: next step while grounded must take off again.
    if(game.player().grounded)
    {
        game.step(jump, kTick);
    }
    EXPECT_FALSE(game.player().grounded);
    EXPECT_GT(game.player().vel.y, 0.f);
}

TEST(TowerGameShould, wallBounceReversesVx)
{
    Tuning tuning;
    tuning.startPlatformW = kWorldW;
    tuning.iceFriction = 0.f;
    tuning.gravity = 0.f;
    tuning.moveAccel = 100000.f;
    tuning.maxRunSpeed = 400.f;
    tuning.wallBounce = 0.95f;
    TowerGame game{1, tuning};

    SimInput left;
    left.left = true;
    bool bounced = false;
    for(int i = 0; i < 120 && !game.over(); ++i)
    {
        const std::vector<TowerEvent> events = game.step(left, kTick);
        for(const TowerEvent& event: events)
        {
            if(std::holds_alternative<WallBounced>(event))
            {
                bounced = true;
            }
        }
        if(bounced)
        {
            break;
        }
    }

    EXPECT_TRUE(bounced);
    EXPECT_FLOAT_EQ(game.player().pos.x, 0.f);
    EXPECT_GT(game.player().vel.x, 0.f);
}

TEST(TowerGameShould, heightRaisesRunSpeedCap)
{
    Tuning tuning;
    tuning.gravity = 0.f;
    tuning.jumpSpeed = 1000.f;
    tuning.heightAccel = 0.00015f;
    tuning.heightAccelCap = 0.55f;
    tuning.iceFriction = 0.f;
    TowerGame game{1, tuning};
    const float baseCap = game.runSpeedCap();
    EXPECT_FLOAT_EQ(baseCap, tuning.maxRunSpeed);

    SimInput jump;
    jump.jump = true;
    game.step(jump, Seconds{1.f});
    EXPECT_GT(game.player().pos.y, 0.f);
    EXPECT_GT(game.runSpeedCap(), baseCap);
    EXPECT_LT(game.runSpeedCap(), tuning.maxRunSpeed * (1.f + tuning.heightAccelCap) + 0.01f);
}

[[nodiscard]] bool chaseBonus(TowerGame& game, BonusKind want, int maxSteps)
{
    for(int i = 0; i < maxSteps && !game.over(); ++i)
    {
        const BonusView* target = nullptr;
        for(const BonusView& bonus: game.bonuses())
        {
            if(bonus.kind == want)
            {
                target = &bonus;
                break;
            }
        }
        if(target == nullptr)
        {
            return false;
        }

        SimInput move;
        const float playerCenter = game.player().pos.x + game.player().size.x * 0.5f;
        const float bonusCenter = target->pos.x + target->size.x * 0.5f;
        move.right = bonusCenter >= playerCenter;
        move.left = !move.right;
        move.jump = game.player().grounded;

        const std::vector<TowerEvent> events = game.step(move, kTick);
        for(const TowerEvent& event: events)
        {
            if(const auto* picked = std::get_if<BonusPicked>(&event))
            {
                if(picked->kind == want)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

TEST(TowerGameShould, collectingPointsBonusAddsScore)
{
    Tuning tuning;
    tuning.bonusChance = 1.f;
    tuning.pointsBonus = 50;
    tuning.gravity = 0.f;
    tuning.iceFriction = 0.f;
    tuning.startPlatformW = kWorldW;
    tuning.moveAccel = 100000.f;
    tuning.maxRunSpeed = 2000.f;
    tuning.bandHeight = 60.f;
    tuning.jumpSpeed = 90.f;
    tuning.heightAccel = 0.f;

    bool found = false;
    for(std::uint64_t seed = 1; seed < 300 && !found; ++seed)
    {
        TowerGame game{seed, tuning};
        bool hasPoints = false;
        for(const BonusView& bonus: game.bonuses())
        {
            hasPoints = hasPoints || bonus.kind == BonusKind::Points;
        }
        if(!hasPoints)
        {
            continue;
        }
        const int scoreAtStart = game.score();
        bool sawPoints = false;
        int pointsAwarded = 0;
        TowerGame again{seed, tuning};
        for(int i = 0; i < 400 && !again.over() && !sawPoints; ++i)
        {
            const BonusView* target = nullptr;
            for(const BonusView& bonus: again.bonuses())
            {
                if(bonus.kind == BonusKind::Points)
                {
                    target = &bonus;
                    break;
                }
            }
            if(target == nullptr)
            {
                break;
            }
            SimInput move;
            const float playerCenter = again.player().pos.x + again.player().size.x * 0.5f;
            const float bonusCenter = target->pos.x + target->size.x * 0.5f;
            move.right = bonusCenter >= playerCenter;
            move.left = !move.right;
            move.jump = again.player().grounded;
            const int before = again.score();
            const std::vector<TowerEvent> events = again.step(move, kTick);
            for(const TowerEvent& event: events)
            {
                if(const auto* picked = std::get_if<BonusPicked>(&event))
                {
                    if(picked->kind == BonusKind::Points)
                    {
                        EXPECT_EQ(picked->points, tuning.pointsBonus);
                        pointsAwarded = picked->points;
                        EXPECT_GE(again.score(), before + pointsAwarded);
                        sawPoints = true;
                    }
                }
            }
        }
        ASSERT_TRUE(sawPoints);
        EXPECT_EQ(pointsAwarded, tuning.pointsBonus);
        EXPECT_GE(again.score(), scoreAtStart + tuning.pointsBonus);
        found = true;
    }
    EXPECT_TRUE(found);
}

TEST(TowerGameShould, speedBoostRaisesRunCap)
{
    Tuning tuning;
    tuning.bonusChance = 1.f;
    tuning.speedBoostMul = 1.45f;
    tuning.boostDuration = 4.f;
    tuning.gravity = 0.f;
    tuning.iceFriction = 0.f;
    tuning.startPlatformW = kWorldW;
    tuning.moveAccel = 100000.f;
    tuning.maxRunSpeed = 320.f;
    tuning.bandHeight = 60.f;
    tuning.jumpSpeed = 90.f;
    tuning.heightAccel = 0.f;

    bool found = false;
    for(std::uint64_t seed = 1; seed < 300 && !found; ++seed)
    {
        TowerGame game{seed, tuning};
        bool hasSpeed = false;
        for(const BonusView& bonus: game.bonuses())
        {
            hasSpeed = hasSpeed || bonus.kind == BonusKind::Speed;
        }
        if(!hasSpeed)
        {
            continue;
        }
        ASSERT_TRUE(chaseBonus(game, BonusKind::Speed, 400));
        EXPECT_NEAR(game.runSpeedCap(), tuning.maxRunSpeed * tuning.speedBoostMul, 0.01f);
        found = true;
    }
    EXPECT_TRUE(found);
}
