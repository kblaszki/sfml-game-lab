#pragma once

#include <asteroids/sim/Types.hpp>
#include <variant>

namespace sgl::asteroids
{

struct ShotFired
{
};

struct RockHit
{
    RockSize size{};
    int score{};
};

struct Died
{
};

struct WaveCleared
{
};

using AsteroidsEvent = std::variant<ShotFired, RockHit, Died, WaveCleared>;

} // namespace sgl::asteroids
