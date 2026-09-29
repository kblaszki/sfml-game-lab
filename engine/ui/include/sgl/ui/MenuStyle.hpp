#pragma once

#include <sgl/core/Color.hpp>
#include <sgl/render/AssetIds.hpp>

namespace sgl
{

struct MenuStyle
{
    FontId font;
    unsigned textSize;
    Color idle;
    Color focused;
    Color text;
    Color panel;
    float cornerRadius{0.f};
    FontId bodyFont{};
};

} // namespace sgl
