#pragma once

#include <tower/sim/Types.hpp>
#include <variant>

namespace sgl::tower
{

struct Landed
{
};

struct Died
{
};

struct WallBounced
{
};

struct BonusPicked
{
    BonusKind kind{};
    int points{};
};

using TowerEvent = std::variant<Landed, Died, WallBounced, BonusPicked>;

} // namespace sgl::tower
