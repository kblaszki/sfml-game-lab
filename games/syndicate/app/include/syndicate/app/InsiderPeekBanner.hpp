#pragma once

#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/GameState.hpp>

namespace sgl::syndicate
{

/// Shows `GameState::revealed` after an Insider peek.
class InsiderPeekBanner
{
public:
    void render(sgl::RenderQueue& queue, const GameState& state, sgl::FontId font, const Theme& theme) const;

    [[nodiscard]] bool visible(const GameState& state) const noexcept;
};

} // namespace sgl::syndicate
