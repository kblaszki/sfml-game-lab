#pragma once

#include <variant>

namespace sgl::tower
{

struct Landed
{
};

struct Died
{
};

using TowerEvent = std::variant<Landed, Died>;

} // namespace sgl::tower
