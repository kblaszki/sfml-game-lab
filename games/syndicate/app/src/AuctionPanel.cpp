#include <syndicate/app/AuctionPanel.hpp>

#include <cstdint>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <sstream>
#include <string>
#include <syndicate/app/Describe.hpp>
#include <syndicate/sim/Auction.hpp>
#include <variant>

namespace sgl::syndicate
{
namespace
{

constexpr sgl::Rect<float> kPanel{.pos = {420.f, 120.f}, .size = {440.f, 200.f}};

[[nodiscard]] std::string playerTag(PlayerId id)
{
    return "P" + std::to_string(static_cast<int>(id) + 1);
}

[[nodiscard]] std::string tileTitle(const GameState& state, NodeId tile)
{
    const std::string full = describeTile(state, tile);
    const auto dash = full.find(" — ");
    if(dash == std::string::npos)
    {
        return full;
    }
    return full.substr(0, dash);
}

} // namespace

bool AuctionPanel::visible(const Phase& phase) const noexcept
{
    return std::holds_alternative<InAuction>(phase);
}

void AuctionPanel::render(sgl::RenderQueue& queue, const Match& match, sgl::FontId font, const Theme& theme) const
{
    const GameState& state = match.state();
    const auto* auctionPhase = std::get_if<InAuction>(&state.phase);
    if(auctionPhase == nullptr)
    {
        return;
    }
    const AuctionState& a = auctionPhase->auction;

    float depth = 80.f;
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(kPanel, 12.f, sgl::Color{20, 24, 40, 235}));
    queue.push(sgl::Layer::Overlay, depth++,
               sgl::roundedRectOutline(kPanel, 12.f, 2.f, sgl::Color{220, 200, 90, 240}));

    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Auction",
                   .size = 22,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 16.f},
                   .color = sgl::Color{245, 230, 140},
               });

    std::ostringstream lot;
    lot << tileTitle(state, a.tile) << "  x" << static_cast<int>(a.shares) << " shares";
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = lot.str(),
                   .size = 16,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 52.f},
                   .color = sgl::Color{230, 235, 245},
               });

    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "High bid  " + formatMoney(a.highBid),
                   .size = 18,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 84.f},
                   .color = sgl::Color{255, 245, 200},
               });

    const std::string leader =
        a.leader.has_value() ? ("Leader  " + playerTag(*a.leader)) : "Leader  —";
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = leader,
                   .size = 16,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 116.f},
                   .color = a.leader.has_value()
                                ? theme.pawn[static_cast<std::size_t>(*a.leader)]
                                : sgl::Color{180, 185, 200},
               });

    const sgl::Color actColor = theme.pawn[static_cast<std::size_t>(a.toAct)];
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "To act  " + playerTag(a.toAct),
                   .size = 18,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 148.f},
                   .color = actColor,
               });

    std::ostringstream active;
    active << "In: ";
    bool first = true;
    for(int i = 0; i < a.seats; ++i)
    {
        if(!a.active[static_cast<std::size_t>(i)])
        {
            continue;
        }
        if(!first)
        {
            active << ", ";
        }
        first = false;
        active << 'P' << (i + 1);
    }
    if(first)
    {
        active << "—";
    }
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = active.str(),
                   .size = 14,
                   .position = {kPanel.pos.x + 20.f, kPanel.pos.y + 176.f},
                   .color = sgl::Color{170, 180, 200},
               });
}

} // namespace sgl::syndicate
