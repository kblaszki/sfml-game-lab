#pragma once

#include <sgl/core/Time.hpp>

namespace sgl::snake
{

struct Tuning
{
    sgl::Seconds moveInterval{0.15f};
    int startLength{3};
};

} // namespace sgl::snake
