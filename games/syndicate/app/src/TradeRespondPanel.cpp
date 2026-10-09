#include <cstdint>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <sstream>
#include <string>
#include <syndicate/app/Describe.hpp>
#include <syndicate/app/TradeRespondPanel.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Trade.hpp>
#include <utility>
#include <variant>

namespace sgl::syndicate
{
namespace
{

constexpr sgl::Rect<float> kPanel{.pos = {420.f, 140.f}, .size = {440.f, 240.f}};

[[nodiscard]] std::string playerTag(PlayerId id)
{
    return "P" + std::to_string(static_cast<int>(id) + 1);
}

[[nodiscard]] std::string tileLabel(const GameState& state, NodeId tile)
{
    const TileNode& node = state.board.node(tile);
    if(const auto* sector = std::get_if<SectorTile>(&node.kind))
    {
        return std::string{sectorName(sector->sector)};
    }
    return "tile " + std::to_string(static_cast<int>(std::to_underlying(tile)));
}

[[nodiscard]] std::string lotsSummary(const GameState& state, const std::vector<ShareLot>& lots)
{
    if(lots.empty())
    {
        return "none";
    }
    std::ostringstream out;
    for(std::size_t i = 0; i < lots.size(); ++i)
    {
        if(i != 0)
        {
            out << ", ";
        }
        out << tileLabel(state, lots[i].tile) << " x" << static_cast<int>(lots[i].count);
    }
    return out.str();
}

} // namespace

void TradeRespondPanel::render(sgl::RenderQueue& queue, const Match& match, sgl::FontId font, const Theme& theme) const
{
    const GameState& state = match.state();
    const auto* pending = std::get_if<AwaitTradeResponse>(&state.phase);
    if(pending == nullptr)
    {
        return;
    }
    const TradeOffer& offer = pending->offer;

    float depth = 80.f;
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(kPanel, 12.f, sgl::Color{20, 24, 40, 235}));
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRectOutline(kPanel, 12.f, 2.f, sgl::Color{220, 200, 90, 240}));

    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Trade offer",
                   .size = 22,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 16.f},
                   .color = sgl::Color{245, 230, 140},
               });

    const sgl::Color fromColor = theme.pawn[static_cast<std::size_t>(offer.from) % 4];
    const sgl::Color toColor = theme.pawn[static_cast<std::size_t>(offer.to) % 4];
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = playerTag(offer.from) + "  offers  " + playerTag(offer.to),
                   .size = 18,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 56.f},
                   .color = fromColor,
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Pays  " + formatMoney(offer.cashFrom),
                   .size = 16,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 92.f},
                   .color = sgl::Color{255, 245, 200},
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Asks  " + formatMoney(offer.cashTo),
                   .size = 16,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 118.f},
                   .color = toColor,
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Gives  " + lotsSummary(state, offer.sharesFrom),
                   .size = 15,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 154.f},
                   .color = sgl::Color{220, 225, 235},
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Gets  " + lotsSummary(state, offer.sharesTo),
                   .size = 15,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 180.f},
                   .color = sgl::Color{220, 225, 235},
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Accept or Reject on the bar",
                   .size = 14,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 208.f},
                   .color = sgl::Color{170, 180, 200},
               });
}

} // namespace sgl::syndicate
