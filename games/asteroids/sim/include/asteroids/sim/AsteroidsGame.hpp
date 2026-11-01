#pragma once

#include <asteroids/sim/Events.hpp>
#include <asteroids/sim/Tuning.hpp>
#include <asteroids/sim/Types.hpp>
#include <cstdint>
#include <sgl/core/Random.hpp>
#include <sgl/core/Time.hpp>
#include <vector>

namespace sgl::asteroids
{

class AsteroidsGame
{
public:
    explicit AsteroidsGame(std::uint64_t seed, Tuning tuning = {});

    std::vector<AsteroidsEvent> step(const SimInput& input, sgl::Seconds dt);

    [[nodiscard]] ShipView ship() const noexcept;
    [[nodiscard]] const std::vector<BulletView>& bullets() const noexcept;
    [[nodiscard]] const std::vector<RockView>& rocks() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] bool over() const noexcept;

private:
    struct Ship
    {
        Vec2f pos{};
        Vec2f vel{};
        float heading{};
    };

    struct Bullet
    {
        Vec2f pos{};
        Vec2f vel{};
        float age{};
    };

    struct Rock
    {
        Vec2f pos{};
        Vec2f vel{};
        RockSize size{};
    };

    void spawnWave();
    void turnShip(const SimInput& input, float seconds);
    void thrustShip(bool thrust, float seconds);
    void updateFire(const SimInput& input, float seconds, std::vector<AsteroidsEvent>& events);
    void advanceBullets(float seconds);
    void advanceRocks(float seconds);
    void hitRocks(std::vector<AsteroidsEvent>& events);
    void hitShip(std::vector<AsteroidsEvent>& events);
    void spawnChildren(const Rock& parent, std::vector<Rock>& out);
    void rebuildViews();
    [[nodiscard]] float radiusOf(RockSize size) const noexcept;
    [[nodiscard]] int scoreOf(RockSize size) const noexcept;

    Tuning tuning_{};
    sgl::Pcg32 rng_;
    Ship ship_{};
    std::vector<Bullet> bullets_{};
    std::vector<Rock> rocks_{};
    int score_{0};
    bool over_{false};
    float fireCooldownLeft_{0.f};
    std::vector<BulletView> bulletViews_{};
    std::vector<RockView> rockViews_{};
};

} // namespace sgl::asteroids
