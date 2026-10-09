#pragma once

#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/Match.hpp>

namespace sgl::syndicate
{

/// Status card for `AwaitTradeResponse`. Accept and Reject stay on the ActionBar.
class TradeRespondPanel
{
public:
    void render(sgl::RenderQueue& queue, const Match& match, sgl::FontId font, const Theme& theme) const;
};

} // namespace sgl::syndicate
