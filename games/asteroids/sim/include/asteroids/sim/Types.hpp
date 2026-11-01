#pragma once

#include <cstdint>
#include <sgl/core/Vec2.hpp>

namespace sgl::asteroids
{

using Vec2f = sgl::Vec2<float>;

inline constexpr float kWorldW = 1280.f;
inline constexpr float kWorldH = 720.f;

enum class RockSize : std::uint8_t
{
    Large,
    Medium,
    Small
};

struct SimInput
{
    bool thrust{};
    bool rotateCw{};
    bool rotateCcw{};
    bool fire{};
};

struct ShipView
{
    Vec2f pos{};
    Vec2f vel{};
    float heading{};
    float radius{};
};

struct BulletView
{
    Vec2f pos{};
    float radius{};
};

struct RockView
{
    Vec2f pos{};
    RockSize size{};
    float radius{};
};

} // namespace sgl::asteroids
