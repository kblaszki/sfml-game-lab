#pragma once

#include <variant>

namespace sgl::dodger
{

struct HazardCleared
{
};

struct Died
{
};

using DodgerEvent = std::variant<HazardCleared, Died>;

} // namespace sgl::dodger
