#pragma once

#include <cstdint>
#include <optional>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/DiceAnimator.hpp>
#include <syndicate/app/PawnAnimator.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/Ids.hpp>
#include <vector>

namespace sgl::syndicate
{

/// Sequences dice tumble then pawn path; cosmetic RNG only (not Match RNG).
class Presentation
{
public:
    explicit Presentation(PawnAnimator& animator, std::uint64_t seed);

    void onRolled(int roll);
    void onPath(PlayerId id, const std::vector<sgl::Vec2f>& points);
    void update(sgl::Seconds dt);
    [[nodiscard]] bool busy() const noexcept;
    void fastForward();
    void render(sgl::RenderQueue& queue, sgl::Vec2f diceCenter, const Theme& theme) const;

private:
    struct QueuedPath
    {
        PlayerId id{};
        std::vector<sgl::Vec2f> points;
    };

    void startQueuedPath();

    PawnAnimator* animator_;
    DiceAnimator dice_;
    std::optional<QueuedPath> queued_;
};

} // namespace sgl::syndicate
