#include <cstdint>
#include <optional>
#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <span>
#include <sstream>
#include <string>
#include <syndicate/app/Describe.hpp>
#include <syndicate/app/TradeComposeOverlay.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Commands.hpp>
#include <utility>
#include <variant>

namespace sgl::syndicate
{
namespace
{

constexpr sgl::Rect<float> kPanel{.pos = {16.f, 80.f}, .size = {308.f, 220.f}};
constexpr sgl::Rect<float> kConfirm{.pos = {28.f, 248.f}, .size = {136.f, 36.f}};
constexpr sgl::Rect<float> kCancel{.pos = {176.f, 248.f}, .size = {136.f, 36.f}};
constexpr Money kCashStep{25};

[[nodiscard]] std::string playerTag(PlayerId id)
{
    return "P" + std::to_string(static_cast<int>(id) + 1);
}

[[nodiscard]] std::size_t playerIndex(PlayerId id) noexcept
{
    return static_cast<std::size_t>(std::to_underlying(id));
}

[[nodiscard]] PlayerId firstOther(const Match& match, PlayerId from)
{
    const int seats = match.state().seats;
    for(int i = 0; i < seats; ++i)
    {
        const PlayerId id{static_cast<std::uint8_t>(i)};
        if(id != from)
        {
            return id;
        }
    }
    return from;
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

void cyclePartner(PlayerId from, PlayerId& to, int seats, int direction)
{
    std::vector<PlayerId> others;
    others.reserve(static_cast<std::size_t>(seats));
    for(int i = 0; i < seats; ++i)
    {
        const PlayerId id{static_cast<std::uint8_t>(i)};
        if(id != from)
        {
            others.push_back(id);
        }
    }
    if(others.empty())
    {
        return;
    }

    std::size_t index = 0;
    for(std::size_t i = 0; i < others.size(); ++i)
    {
        if(others[i] == to)
        {
            index = i;
            break;
        }
    }
    if(direction >= 0)
    {
        index = (index + 1) % others.size();
    }
    else
    {
        index = (index + others.size() - 1) % others.size();
    }
    to = others[index];
}

void toggleOwnedShare(std::vector<ShareLot>& lots, NodeId tile)
{
    for(auto it = lots.begin(); it != lots.end(); ++it)
    {
        if(it->tile == tile)
        {
            lots.erase(it);
            return;
        }
    }
    lots.push_back(ShareLot{.tile = tile, .count = 1});
}

[[nodiscard]] sgl::Vec2f rectCenter(sgl::Rect<float> rect)
{
    return {rect.pos.x + rect.size.x * 0.5f, rect.pos.y + rect.size.y * 0.5f};
}

} // namespace

void TradeComposeOverlay::open(const Match& match, PlayerId from)
{
    active_ = true;
    from_ = from;
    to_ = firstOther(match, from);
    cashFrom_ = Money{0};
    sharesFrom_.clear();
    sharesLabel_ = "none";
}

void TradeComposeOverlay::close()
{
    active_ = false;
}

bool TradeComposeOverlay::active() const noexcept
{
    return active_;
}

sgl::Rect<float> TradeComposeOverlay::confirmRect() const noexcept
{
    return kConfirm;
}

sgl::Rect<float> TradeComposeOverlay::cancelRect() const noexcept
{
    return kCancel;
}

void TradeComposeOverlay::update(const sgl::InputState& input,
                                 Match& match,
                                 const LayoutView& layout,
                                 const SyndicateActions& actions)
{
    if(!active_)
    {
        return;
    }

    const GameState& state = match.state();
    if(input.action(actions.menu.left).pressed)
    {
        cyclePartner(from_, to_, state.seats, -1);
    }
    if(input.action(actions.menu.right).pressed)
    {
        cyclePartner(from_, to_, state.seats, +1);
    }

    const Money cash = state.cash[playerIndex(from_)];
    if(input.action(actions.menu.up).pressed)
    {
        const Money next = cashFrom_ + kCashStep;
        if(!(next > cash))
        {
            cashFrom_ = next;
        }
    }
    if(input.action(actions.menu.down).pressed)
    {
        if(cashFrom_.v >= kCashStep.v)
        {
            cashFrom_ = cashFrom_ - kCashStep;
        }
        else
        {
            cashFrom_ = Money{0};
        }
    }

    if(!input.pointerReleased())
    {
        return;
    }
    const auto pointer = input.pointer();
    if(!pointer.has_value())
    {
        return;
    }

    if(kConfirm.contains(*pointer))
    {
        const TradeOffer offer{
            .from = from_,
            .to = to_,
            .cashFrom = cashFrom_,
            .cashTo = Money{0},
            .sharesFrom = sharesFrom_,
            .sharesTo = {},
        };
        const auto sharesOf = [&state](PlayerId who, NodeId tile) -> std::uint8_t {
            return state.ownership.holdings(tile).shares[playerIndex(who)];
        };
        if(validateTrade(offer, std::span<const Money>{state.cash}, sharesOf))
        {
            if(match.apply(from_, ProposeTrade{offer}))
            {
                close();
            }
        }
        return;
    }

    if(kCancel.contains(*pointer))
    {
        close();
        return;
    }

    const float maxDist = layout.nodeRadius() * 1.5f;
    const float maxDistSq = maxDist * maxDist;
    std::optional<NodeId> hit;
    float bestDistSq = maxDistSq;
    for(std::size_t i = 0; i < state.board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        if(!std::holds_alternative<SectorTile>(state.board.node(id).kind))
        {
            continue;
        }
        if(state.ownership.holdings(id).shares[playerIndex(from_)] < 1)
        {
            continue;
        }
        const sgl::Vec2f center = layout.position(state.board.node(id).at);
        const float distSq = sgl::lengthSq(*pointer - center);
        if(distSq <= bestDistSq)
        {
            bestDistSq = distSq;
            hit = id;
        }
    }
    if(hit.has_value())
    {
        toggleOwnedShare(sharesFrom_, *hit);
        sharesLabel_ = lotsSummary(state, sharesFrom_);
    }
}

void TradeComposeOverlay::render(sgl::RenderQueue& queue, sgl::FontId font, const Theme& theme) const
{
    if(!active_)
    {
        return;
    }

    float depth = 90.f;
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(kPanel, 12.f, sgl::Color{20, 24, 40, 235}));
    queue.push(
        sgl::Layer::Overlay, depth++, sgl::roundedRectOutline(kPanel, 12.f, 2.f, sgl::Color{140, 190, 220, 240}));

    const sgl::Color toColor = theme.pawn[playerIndex(to_) % 4];
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Propose trade",
                   .size = 20,
                   .position = {kPanel.pos.x + 16.f, kPanel.pos.y + 14.f},
                   .color = sgl::Color{220, 235, 245},
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "To  " + playerTag(to_),
                   .size = 16,
                   .position = {kPanel.pos.x + 16.f, kPanel.pos.y + 48.f},
                   .color = toColor,
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "You pay  " + formatMoney(cashFrom_),
                   .size = 16,
                   .position = {kPanel.pos.x + 16.f, kPanel.pos.y + 78.f},
                   .color = sgl::Color{255, 245, 200},
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Shares  " + sharesLabel_,
                   .size = 14,
                   .position = {kPanel.pos.x + 16.f, kPanel.pos.y + 108.f},
                   .color = sgl::Color{210, 215, 225},
               });

    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(kConfirm, 6.f, sgl::Color{40, 90, 70, 240}));
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Confirm",
                   .size = 16,
                   .position = rectCenter(kConfirm),
                   .color = sgl::Color{235, 245, 235},
                   .anchor = sgl::Anchor::Center,
               });
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(kCancel, 6.f, sgl::Color{90, 48, 48, 240}));
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = "Cancel",
                   .size = 16,
                   .position = rectCenter(kCancel),
                   .color = sgl::Color{245, 230, 230},
                   .anchor = sgl::Anchor::Center,
               });
}

} // namespace sgl::syndicate
