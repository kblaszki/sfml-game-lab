#pragma once

#include <variant>

namespace sgl::tappy
{

struct Flapped
{
};

struct GateCleared
{
    int gateIndex{};
};

struct Died
{
};

using TappyEvent = std::variant<Flapped, GateCleared, Died>;

} // namespace sgl::tappy
