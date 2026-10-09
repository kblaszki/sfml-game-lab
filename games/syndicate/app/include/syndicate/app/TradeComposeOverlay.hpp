#pragma once

#include <sgl/input/InputState.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <string>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Match.hpp>
#include <syndicate/sim/Money.hpp>
#include <syndicate/sim/Trade.hpp>
#include <vector>

namespace sgl::syndicate
{

/// Hot-seat trade draft. Builds a `TradeOffer` and applies `ProposeTrade` on Confirm.
class TradeComposeOverlay
{
public:
    void open(const Match& match, PlayerId from);
    void close();
    [[nodiscard]] bool active() const noexcept;

    void update(const sgl::InputState& input, Match& match, const LayoutView& layout, const SyndicateActions& actions);

    void render(sgl::RenderQueue& queue, sgl::FontId font, const Theme& theme) const;

    [[nodiscard]] sgl::Rect<float> confirmRect() const noexcept;
    [[nodiscard]] sgl::Rect<float> cancelRect() const noexcept;

private:
    bool active_{false};
    PlayerId from_{};
    PlayerId to_{};
    Money cashFrom_{};
    std::vector<ShareLot> sharesFrom_{};
    std::string sharesLabel_{"none"};
};

} // namespace sgl::syndicate
