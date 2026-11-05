#pragma once

#include <cstdint>
#include <sgl/core/Random.hpp>
#include <sgl/core/Time.hpp>
#include <tappy/sim/Events.hpp>
#include <tappy/sim/Tuning.hpp>
#include <tappy/sim/Types.hpp>
#include <vector>

namespace sgl::tappy
{

class TappyGame
{
public:
    explicit TappyGame(std::uint64_t seed, Tuning tuning = {});

    std::vector<TappyEvent> step(const SimInput& input, sgl::Seconds dt);

    [[nodiscard]] PlayerView player() const noexcept;
    [[nodiscard]] const std::vector<GateView>& gates() const noexcept;
    [[nodiscard]] float cameraLeft() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] bool over() const noexcept;
    [[nodiscard]] float scrollSpeed() const noexcept;
    [[nodiscard]] float distance() const noexcept;

private:
    void ensureGates();
    void cullGates();
    void clearGates(std::vector<TappyEvent>& events);
    void collide(std::vector<TappyEvent>& events);
    [[nodiscard]] float gapAt(float worldX) const noexcept;
    [[nodiscard]] float scrollSpeedAt(float worldX) const noexcept;
    void die(std::vector<TappyEvent>& events);

    Tuning tuning_{};
    sgl::Pcg32 rng_;
    PlayerView player_{};
    std::vector<GateView> gates_{};
    float nextGateX_{0.f};
    float distance_{0.f};
    int score_{0};
    int nextGateIndex_{0};
    bool over_{false};
};

} // namespace sgl::tappy
