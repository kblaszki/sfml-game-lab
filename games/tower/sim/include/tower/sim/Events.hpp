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

struct EdgeJumped
{
};

using TowerEvent = std::variant<Landed, Died, WallBounced, BonusPicked, EdgeJumped>;

} // namespace sgl::tower
