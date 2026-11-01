#pragma once

#include <sgl/core/Vec2.hpp>

namespace sgl::dodger
{

using Vec2f = sgl::Vec2<float>;

inline constexpr float kWorldW = 1280.f;
inline constexpr float kWorldH = 720.f;
inline constexpr float kPaddleY = 660.f;

struct SimInput
{
    bool left{};
    bool right{};
};

struct PaddleView
{
    Vec2f pos{};
    Vec2f size{};
};

struct HazardView
{
    Vec2f pos{};
    Vec2f size{};
};

} // namespace sgl::dodger
