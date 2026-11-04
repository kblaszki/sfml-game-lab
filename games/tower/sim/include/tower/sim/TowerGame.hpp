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
    [[nodiscard]] float cameraBottom() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] bool over() const noexcept;

private:
    void spawnStart();
    void spawnBand(int band);
    void ensurePlatforms();
    void cullPlatforms();
    void notePeak() noexcept;
    void tryLand(float previousFeet, std::vector<TowerEvent>& events);
    [[nodiscard]] float widthAt(float surfaceY) const noexcept;
    [[nodiscard]] bool overlapsX(const PlatformView& platform) const noexcept;
    [[nodiscard]] bool supported() const noexcept;

    Tuning tuning_{};
    sgl::Pcg32 rng_;
    PlayerView player_{};
    std::vector<PlatformView> platforms_{};
    float cameraBottom_{0.f};
    float peakFeet_{0.f};
    int nextBand_{1};
    int score_{0};
    bool over_{false};
};

} // namespace sgl::tower
