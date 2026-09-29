#pragma once

#include <sgl/core/Rect.hpp>
#include <string>

namespace sgl
{

struct Button
{
    Rect<float> rect;
    std::string label;
};

} // namespace sgl
