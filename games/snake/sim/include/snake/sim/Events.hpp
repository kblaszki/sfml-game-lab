#pragma once

#include <variant>

namespace sgl::snake
{

struct Moved
{
};

struct Ate
{
    int score{};
};

struct Died
{
};

using SnakeEvent = std::variant<Moved, Ate, Died>;

} // namespace sgl::snake
