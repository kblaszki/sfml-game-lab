#pragma once

#include <cstdint>
#include <dodger/sim/Events.hpp>
#include <dodger/sim/Tuning.hpp>
#include <dodger/sim/Types.hpp>
#include <sgl/core/Random.hpp>
#include <sgl/core/Time.hpp>
#include <vector>

namespace sgl::dodger
{

class DodgerGame
{
public:
    explicit DodgerGame(std::uint64_t seed, Tuning tuning = {});

    std::vector<DodgerEvent> step(const SimInput& input, sgl::Seconds dt);

    [[nodiscard]] PaddleView paddle() const noexcept;
    [[nodiscard]] const std::vector<HazardView>& hazards() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] bool over() const noexcept;

private:
    Tuning tuning_{};
    sgl::Pcg32 rng_;
    PaddleView paddle_{};
    std::vector<HazardView> hazards_{};
    int score_{0};
    bool over_{false};
    float spawnAccum_{0.f};
};

} // namespace sgl::dodger
