#pragma once

#include <cstdint>
#include <sgl/core/Vec2.hpp>

namespace sgl::tower
{

using Vec2f = sgl::Vec2<float>;

inline constexpr float kWorldW = 1280.f;
inline constexpr float kViewH = 720.f;

enum class BonusKind : std::uint8_t
{
    Speed,
    Jump,
    Points
};

struct SimInput
{
    bool left{};
    bool right{};
    bool jump{};
};

struct PlayerView
{
    Vec2f pos{};
    Vec2f vel{};
    Vec2f size{};
    bool grounded{};
};

struct PlatformView
{
    Vec2f pos{};
    Vec2f size{};
};

struct BonusView
{
    Vec2f pos{};
    Vec2f size{};
    BonusKind kind{};
};

} // namespace sgl::tower
