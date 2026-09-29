#pragma once

#include <array>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ids.hpp>
#include <vector>

namespace sgl::syndicate
{

/// Constant-speed polyline motion for up to four pawns.
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

private:
    struct State
    {
        sgl::Vec2f pos{};
        std::vector<sgl::Vec2f> points;
        std::size_t segment{};
        float along{};
        bool animating{};
    };

    const LayoutView* layout_;
    float speed_{400.f};
    std::array<State, kMaxPlayers> states_{};
};

} // namespace sgl::syndicate
