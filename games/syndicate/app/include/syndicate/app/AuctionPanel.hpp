#pragma once

#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Match.hpp>

namespace sgl::syndicate
{

/// Centered status panel for `InAuction`. Reads `AuctionState` only; no input.
class AuctionPanel
{
public:
    void render(sgl::RenderQueue& queue, const Match& match, sgl::FontId font, const Theme& theme) const;

    [[nodiscard]] bool visible(const Phase& phase) const noexcept;
};

} // namespace sgl::syndicate
