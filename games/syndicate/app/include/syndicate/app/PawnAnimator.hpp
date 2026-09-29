#pragma once

#include <array>
#include <sgl/anim/Spline.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ids.hpp>
#include <vector>

namespace sgl::syndicate
{

/// Ease-in-out Catmull-Rom motion for up to four pawns.
/// `position()` always adds `Vec2f{(id - 1.5f) * 12.f, 0}` so seats never stack.
class PawnAnimator
{
public:
    explicit PawnAnimator(const LayoutView& layout);

    void reset(PlayerId id, RingSlot at);
    void start(PlayerId id, const std::vector<sgl::Vec2f>& points);
    void update(sgl::Seconds dt);
    [[nodiscard]] sgl::Vec2f position(PlayerId id) const;
    [[nodiscard]] bool busy() const noexcept;
    void setSpeed(float pixelsPerSecond);
    void finish(PlayerId id);
    void finishAll();

private:
    struct State
    {
        sgl::Vec2f pos{};
        std::vector<sgl::Vec2f> control;
        sgl::ArcLengthTable table{};
        float elapsed{0.f};
        float duration{0.f};
        bool animating{};
    };

    void snapToEnd(State& s);

    const LayoutView* layout_;
    float speed_{400.f};
    std::array<State, kMaxPlayers> states_{};
};

} // namespace sgl::syndicate
