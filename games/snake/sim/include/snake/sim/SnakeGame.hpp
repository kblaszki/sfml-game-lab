#pragma once

#include <cstdint>
#include <optional>
#include <sgl/core/Random.hpp>
#include <sgl/core/Time.hpp>
#include <snake/sim/Events.hpp>
#include <snake/sim/Tuning.hpp>
#include <snake/sim/Types.hpp>
#include <vector>

namespace sgl::snake
{

struct SimInput
{
    std::optional<Dir> turn{}; // nullopt = keep; ignore 180° reverse
};

class SnakeGame
{
public:
    explicit SnakeGame(std::uint64_t seed, Tuning tuning = {});

    std::vector<SnakeEvent> step(const SimInput& input, sgl::Seconds dt);

    [[nodiscard]] const std::vector<Cell>& body() const noexcept; // [0]=head
    [[nodiscard]] Cell food() const noexcept;
    [[nodiscard]] Dir dir() const noexcept;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] bool over() const noexcept;

private:
    void applyTurn(std::optional<Dir> turn);
    void advance(std::vector<SnakeEvent>& events);
    void placeFood();
    [[nodiscard]] bool occupied(Cell cell) const;
    [[nodiscard]] bool hitsSelf(Cell next, bool eating) const;

    Tuning tuning_{};
    sgl::Pcg32 rng_;
    std::vector<Cell> body_{};
    Cell food_{};
    Dir dir_{Dir::Right};
    int score_{0};
    bool over_{false};
    bool hasFood_{false};
    sgl::Seconds moveAccum_{};
};

} // namespace sgl::snake
