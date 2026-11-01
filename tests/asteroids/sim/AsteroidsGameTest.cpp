#include <asteroids/sim/AsteroidsGame.hpp>
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <variant>
#include <vector>

namespace
{

using sgl::Seconds;
using sgl::asteroids::AsteroidsEvent;
using sgl::asteroids::AsteroidsGame;
using sgl::asteroids::BulletView;
using sgl::asteroids::Died;
using sgl::asteroids::kWorldH;
using sgl::asteroids::kWorldW;
using sgl::asteroids::RockHit;
using sgl::asteroids::RockSize;
using sgl::asteroids::RockView;
using sgl::asteroids::ShipView;
using sgl::asteroids::ShotFired;
using sgl::asteroids::SimInput;
using sgl::asteroids::Tuning;
using sgl::asteroids::WaveCleared;

constexpr Seconds kFrame{1.f / 60.f};
constexpr int kDeterminismSteps = 240;

template<typename Event>
[[nodiscard]] int countEvents(const std::vector<AsteroidsEvent>& events)
{
    int count = 0;
    for(const AsteroidsEvent& event: events)
    {
        if(std::holds_alternative<Event>(event))
        {
            ++count;
        }
    }
    return count;
}

void expectMatch(const AsteroidsGame& left, const AsteroidsGame& right)
{
    EXPECT_EQ(left.score(), right.score());
    EXPECT_EQ(left.over(), right.over());

    const ShipView shipLeft = left.ship();
    const ShipView shipRight = right.ship();
    EXPECT_EQ(shipLeft.pos, shipRight.pos);
    EXPECT_EQ(shipLeft.vel, shipRight.vel);
    EXPECT_EQ(shipLeft.heading, shipRight.heading);
    EXPECT_EQ(shipLeft.radius, shipRight.radius);

    const std::vector<BulletView> bulletsLeft = left.bullets();
    const std::vector<BulletView> bulletsRight = right.bullets();
    ASSERT_EQ(bulletsLeft.size(), bulletsRight.size());
    for(std::size_t i = 0; i < bulletsLeft.size(); ++i)
    {
        EXPECT_EQ(bulletsLeft[i].pos, bulletsRight[i].pos);
        EXPECT_EQ(bulletsLeft[i].radius, bulletsRight[i].radius);
    }

    const std::vector<RockView> rocksLeft = left.rocks();
    const std::vector<RockView> rocksRight = right.rocks();
    ASSERT_EQ(rocksLeft.size(), rocksRight.size());
    for(std::size_t i = 0; i < rocksLeft.size(); ++i)
    {
        EXPECT_EQ(rocksLeft[i].pos, rocksRight[i].pos);
        EXPECT_EQ(rocksLeft[i].size, rocksRight[i].size);
        EXPECT_EQ(rocksLeft[i].radius, rocksRight[i].radius);
    }
}

[[nodiscard]] AsteroidsGame runSeed(std::uint64_t seed)
{
    AsteroidsGame game{seed};
    for(int frame = 0; frame < kDeterminismSteps; ++frame)
    {
        SimInput input;
        input.thrust = true;
        input.fire = true;
        if(frame >= 90)
        {
            input.rotateCcw = true;
        }
        game.step(input, kFrame);
    }
    return game;
}

} // namespace

TEST(AsteroidsGameShould, rotateChangesHeadingAndThrustFollowsIt)
{
    Tuning tuning;
    tuning.initialLargeRocks = 0;
    AsteroidsGame game{1, tuning};

    SimInput ccw;
    ccw.rotateCcw = true;
    game.step(ccw, Seconds{0.2f});
    EXPECT_GT(game.ship().heading, 0.f);

    AsteroidsGame clockwise{1, tuning};
    SimInput cw;
    cw.rotateCw = true;
    clockwise.step(cw, Seconds{0.2f});
    EXPECT_LT(clockwise.ship().heading, 0.f);

    const float speedBefore = std::hypot(game.ship().vel.x, game.ship().vel.y);
    SimInput thrust;
    thrust.thrust = true;
    game.step(thrust, Seconds{0.25f});
    const ShipView ship = game.ship();
    const float speed = std::hypot(ship.vel.x, ship.vel.y);
    EXPECT_GT(speed, speedBefore);
    EXPECT_NEAR(ship.vel.x, std::cos(ship.heading) * speed, 1e-4f);
    EXPECT_NEAR(ship.vel.y, std::sin(ship.heading) * speed, 1e-4f);
}

TEST(AsteroidsGameShould, fireSpawnsBulletAndCooldownPreventsMachineGun)
{
    Tuning tuning;
    tuning.initialLargeRocks = 0;
    AsteroidsGame game{1, tuning};
    SimInput input;
    input.fire = true;

    const std::vector<AsteroidsEvent> first = game.step(input, kFrame);
    EXPECT_EQ(countEvents<ShotFired>(first), 1);
    EXPECT_EQ(game.bullets().size(), 1u);

    for(int frame = 0; frame < 12; ++frame)
    {
        const std::vector<AsteroidsEvent> held = game.step(input, kFrame);
        EXPECT_EQ(countEvents<ShotFired>(held), 0);
    }
    EXPECT_EQ(game.bullets().size(), 1u);

    const std::vector<AsteroidsEvent> again = game.step(input, Seconds{0.05f});
    EXPECT_EQ(countEvents<ShotFired>(again), 1);
    EXPECT_EQ(game.bullets().size(), 2u);
}

TEST(AsteroidsGameShould, shipWrapsAcrossWorldEdge)
{
    Tuning tuning;
    tuning.initialLargeRocks = 0;
    tuning.thrustAccel = 5000.f;
    tuning.maxShipSpeed = 5000.f;
    AsteroidsGame game{1, tuning};
    const ShipView start = game.ship();

    SimInput input;
    input.thrust = true;
    game.step(input, Seconds{1.f});

    const float rawX = start.pos.x + 5000.f;
    float expectedX = std::fmod(rawX, kWorldW);
    if(expectedX < 0.f)
    {
        expectedX += kWorldW;
    }
    const ShipView ship = game.ship();
    EXPECT_GE(rawX, kWorldW);
    EXPECT_GE(ship.pos.x, 0.f);
    EXPECT_LT(ship.pos.x, kWorldW);
    EXPECT_NEAR(ship.pos.x, expectedX, 1e-3f);
    EXPECT_NEAR(ship.pos.y, start.pos.y, 1e-3f);
    EXPECT_GT(std::fabs(ship.pos.x - start.pos.x), 1.f);
}

TEST(AsteroidsGameShould, bulletSplitsLargeRockAndScores)
{
    Tuning tuning;
    tuning.initialLargeRocks = 1;
    AsteroidsGame game{7, tuning};
    ASSERT_EQ(game.rocks().size(), 1u);
    ASSERT_EQ(game.rocks()[0].size, RockSize::Large);

    bool sawHit = false;
    for(int frame = 0; frame < 180 && !sawHit; ++frame)
    {
        SimInput input;
        input.fire = frame == 0;
        const std::vector<AsteroidsEvent> events = game.step(input, kFrame);
        for(const AsteroidsEvent& event: events)
        {
            if(const auto* hit = std::get_if<RockHit>(&event))
            {
                EXPECT_EQ(hit->size, RockSize::Large);
                EXPECT_EQ(hit->score, tuning.largeScore);
                sawHit = true;
            }
        }
        EXPECT_EQ(countEvents<Died>(events), 0);
    }

    ASSERT_TRUE(sawHit);
    EXPECT_EQ(game.score(), tuning.largeScore);
    EXPECT_FALSE(game.over());
    EXPECT_TRUE(game.bullets().empty());
    ASSERT_EQ(game.rocks().size(), 2u);
    EXPECT_EQ(game.rocks()[0].size, RockSize::Medium);
    EXPECT_EQ(game.rocks()[1].size, RockSize::Medium);
}

TEST(AsteroidsGameShould, shipRockOverlapEndsTheGame)
{
    Tuning tuning;
    tuning.initialLargeRocks = 1;
    AsteroidsGame game{3, tuning};

    int deaths = 0;
    for(int frame = 0; frame < 360 && !game.over(); ++frame)
    {
        SimInput input;
        input.thrust = true;
        const std::vector<AsteroidsEvent> events = game.step(input, kFrame);
        deaths += countEvents<Died>(events);
    }

    EXPECT_EQ(deaths, 1);
    EXPECT_TRUE(game.over());

    const ShipView frozen = game.ship();
    const int frozenScore = game.score();
    SimInput input;
    input.thrust = true;
    input.fire = true;
    const std::vector<AsteroidsEvent> after = game.step(input, Seconds{1.f});
    EXPECT_TRUE(after.empty());
    EXPECT_EQ(game.score(), frozenScore);
    EXPECT_EQ(game.ship().pos, frozen.pos);
    EXPECT_EQ(game.ship().vel, frozen.vel);
    EXPECT_EQ(game.ship().heading, frozen.heading);
}

TEST(AsteroidsGameShould, clearingRocksStartsAnotherWave)
{
    Tuning tuning;
    tuning.initialLargeRocks = 1;
    tuning.fireCooldown = 0.f;
    tuning.bulletLifetime = 4.f;
    tuning.bulletSpeed = 800.f;
    AsteroidsGame game{11, tuning};

    const int expectedScore = tuning.largeScore + 2 * tuning.mediumScore + 4 * tuning.smallScore;
    bool cleared = false;
    for(int frame = 0; frame < 600 && !cleared; ++frame)
    {
        if(game.over())
        {
            break;
        }
        SimInput input;
        input.fire = true;
        const std::vector<AsteroidsEvent> events = game.step(input, kFrame);
        if(countEvents<WaveCleared>(events) == 1)
        {
            cleared = true;
            EXPECT_EQ(game.score(), expectedScore);
            EXPECT_FALSE(game.over());
            ASSERT_EQ(game.rocks().size(), static_cast<std::size_t>(tuning.initialLargeRocks));
            for(const RockView& rock: game.rocks())
            {
                EXPECT_EQ(rock.size, RockSize::Large);
            }
        }
    }

    EXPECT_FALSE(game.over());
    EXPECT_TRUE(cleared);
}

TEST(AsteroidsGameShould, sameSeedMatchesAfterFixedSteps)
{
    const AsteroidsGame left = runSeed(42);
    const AsteroidsGame right = runSeed(42);
    expectMatch(left, right);
}
