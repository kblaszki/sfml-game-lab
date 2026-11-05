#pragma once

#include <sgl/core/Vec2.hpp>

namespace sgl::tappy
{

using Vec2f = sgl::Vec2<float>;

inline constexpr float kWorldW = 1280.f;
inline constexpr float kWorldH = 720.f;
inline constexpr float kCameraFollowFrac = 0.28f;

struct SimInput
{
    bool flap{};
};

struct PlayerView
{
    Vec2f pos{};
    Vec2f vel{};
    Vec2f size{};
};

struct GateView
{
    Vec2f topPos{};
    Vec2f topSize{};
    Vec2f bottomPos{};
    Vec2f bottomSize{};
    bool cleared{};
};

} // namespace sgl::tappy
