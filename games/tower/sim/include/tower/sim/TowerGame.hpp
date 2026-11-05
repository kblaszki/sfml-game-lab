#pragma once

#include <cstdint>
#include <sgl/core/Random.hpp>
#include <sgl/core/Time.hpp>
#include <tower/sim/Events.hpp>
#include <tower/sim/Tuning.hpp>
#include <tower/sim/Types.hpp>
#include <vector>

namespace sgl::tower
{

class TowerGame
{
public:
    explicit TowerGame(std::uint64_t seed, Tuning tuning = {});

    std::vector<TowerEvent> step(const SimInput& input, sgl::Seconds dt);

    [[nodiscard]] PlayerView player() const noexcept;
    [[nodiscard]] const std::vector<PlatformView>& platforms() const noexcept;
    [[nodiscard]] const std::vector<BonusView>& bonuses() const noexcept;
    [[nodiscard]] float cameraBottom() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] int combo() const noexcept;
    [[nodiscard]] bool over() const noexcept;
    [[nodiscard]] float playLeft() const noexcept;
    [[nodiscard]] float playRight() const noexcept;
    [[nodiscard]] float runSpeedCap() const noexcept;
    [[nodiscard]] float jumpSpeedEffective() const noexcept;

private:
    void spawnStart();
    void spawnBand(int band);
    void maybeSpawnBonus(const PlatformView& platform);
    void ensurePlatforms();
    void cullPlatforms();
    void cullBonuses();
    void notePeak() noexcept;
    void tryLand(float previousFeet, std::vector<TowerEvent>& events);
    void bounceWalls(std::vector<TowerEvent>& events);
    void collectBonuses(std::vector<TowerEvent>& events);
    void tickBoosts(float seconds) noexcept;
    void onDied(std::vector<TowerEvent>& events);
    [[nodiscard]] float heightScale() const noexcept;
    [[nodiscard]] float widthAt(float surfaceY) const noexcept;
    [[nodiscard]] bool overlapsX(const PlatformView& platform) const noexcept;
    [[nodiscard]] bool supported() const noexcept;
    [[nodiscard]] const PlatformView* supportPlatform() const noexcept;

    enum class WallSide : std::uint8_t
    {
        None,
        Left,
        Right
    };

    Tuning tuning_{};
    sgl::Pcg32 rng_;
    PlayerView player_{};
    std::vector<PlatformView> platforms_{};
    std::vector<BonusView> bonuses_{};
    float cameraBottom_{0.f};
    float peakFeet_{0.f};
    float speedBoostLeft_{0.f};
    float jumpBoostLeft_{0.f};
    float timeSinceLand_{0.f};
    int nextBand_{1};
    int score_{0};
    int combo_{0};
    WallSide lastBoostWall_{WallSide::None};
    bool over_{false};
};

} // namespace sgl::tower
