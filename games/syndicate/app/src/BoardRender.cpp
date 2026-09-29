#include <cstddef>
#include <cstdint>
#include <set>
#include <sgl/core/Overloaded.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <string>
#include <string_view>
#include <syndicate/app/BoardRender.hpp>
#include <type_traits>
#include <utility>
#include <variant>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] sgl::Color tileFill(const TileKind& kind, const Theme& theme)
{
    return std::visit(
        [&](const auto& tile) -> sgl::Color {
            using T = std::decay_t<decltype(tile)>;
            if constexpr(std::is_same_v<T, SectorTile>)
            {
                return theme.sector[static_cast<std::size_t>(tile.sector)];
            }
            else if constexpr(std::is_same_v<T, Payday>)
            {
                return theme.payday;
            }
            else if constexpr(std::is_same_v<T, Exchange>)
            {
                return theme.exchange;
            }
            else if constexpr(std::is_same_v<T, Hub>)
            {
                return theme.hub;
            }
            else if constexpr(std::is_same_v<T, AuditTile>)
            {
                return theme.audit;
            }
            else if constexpr(std::is_same_v<T, Grant>)
            {
                return theme.grant;
            }
            else
            {
                return theme.edge;
            }
        },
        kind);
}

[[nodiscard]] bool isHighlighted(NodeId id, const std::vector<MoveOption>& highlighted)
{
    for(const auto& opt: highlighted)
    {
        if(opt.destination == id)
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] std::string shortLabel(const TileKind& kind)
{
    return std::visit(
        sgl::Overloaded{
            [](const SectorTile& tile) -> std::string {
                switch(tile.sector)
                {
                case Sector::AI:
                    return "AI";
                case Sector::CleanEnergy:
                    return "CLN";
                case Sector::Biotech:
                    return "BIO";
                case Sector::CyberSec:
                    return "CYB";
                case Sector::Logistics:
                    return "LOG";
                case Sector::Fintech:
                    return "FIN";
                }
                return "???";
            },
            [](const Payday&) -> std::string { return "PAY"; },
            [](const Exchange&) -> std::string { return "EXC"; },
            [](const Hub&) -> std::string { return "HUB"; },
            [](const AuditTile&) -> std::string { return "AUD"; },
            [](const Grant&) -> std::string { return "GRA"; },
        },
        kind);
}

[[nodiscard]] std::string_view spriteName(const TileKind& kind)
{
    return std::visit(
        sgl::Overloaded{
            [](const SectorTile& tile) -> std::string_view {
                switch(tile.sector)
                {
                case Sector::AI:
                    return "sector/ai";
                case Sector::CleanEnergy:
                    return "sector/clean-energy";
                case Sector::Biotech:
                    return "sector/biotech";
                case Sector::CyberSec:
                    return "sector/cybersec";
                case Sector::Logistics:
                    return "sector/logistics";
                case Sector::Fintech:
                    return "sector/fintech";
                }
                return {};
            },
            [](const Payday&) -> std::string_view { return "tile/payday"; },
            [](const Exchange&) -> std::string_view { return "tile/exchange"; },
            [](const Hub&) -> std::string_view { return "tile/hub"; },
            [](const AuditTile&) -> std::string_view { return "tile/audit"; },
            [](const Grant&) -> std::string_view { return "tile/grant"; },
        },
        kind);
}

[[nodiscard]] sgl::Color spriteTint(const TileKind& kind, const Theme& theme)
{
    if(const auto* sector = std::get_if<SectorTile>(&kind))
    {
        return theme.sector[static_cast<std::size_t>(sector->sector)];
    }
    return sgl::Color{255, 255, 255, 255};
}

} // namespace

void renderBoard(sgl::RenderQueue& queue,
                 const BoardGraph& board,
                 const Ownership& ownership,
                 const LayoutView& layout,
                 const Theme& theme,
                 const std::vector<MoveOption>& highlighted,
                 sgl::FontId font,
                 const SpriteBank* sprites)
{
    std::set<std::pair<std::uint16_t, std::uint16_t>> undirected;
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId from{static_cast<std::uint16_t>(i)};
        for(const NodeId to: board.successors(from))
        {
            const auto a = static_cast<std::uint16_t>(from);
            const auto b = static_cast<std::uint16_t>(to);
            if(a < b)
            {
                undirected.emplace(a, b);
            }
            else if(b < a)
            {
                undirected.emplace(b, a);
            }
        }
    }

    float edgeDepth = 0.f;
    for(const auto& [a, b]: undirected)
    {
        const auto& na = board.node(NodeId{a});
        const auto& nb = board.node(NodeId{b});
        queue.push(sgl::Layer::World, edgeDepth++,
                   sgl::LineCmd{.from = layout.position(na.at),
                                .to = layout.position(nb.at),
                                .thickness = 2.f,
                                .color = theme.edge});
    }

    const float r = layout.nodeRadius();
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const auto& node = board.node(id);
        const auto& hold = ownership.holdings(id);
        const bool hi = isHighlighted(id, highlighted);
        const sgl::Vec2f center = layout.position(node.at);

        sgl::Color outline = theme.edge;
        if(hi)
        {
            outline = theme.highlight;
        }
        else if(const auto ctrl = ownership.controller(id))
        {
            outline = theme.pawn[static_cast<std::size_t>(*ctrl)];
        }

        queue.push(sgl::Layer::World, 1000.f + static_cast<float>(i),
                   sgl::CircleCmd{.center = center,
                                  .radius = r,
                                  .fill = tileFill(node.kind, theme),
                                  .outline = outline,
                                  .outlineThickness = static_cast<float>(hold.level) + 1.f,
                                  .points = 32});

        if(sprites != nullptr)
        {
            const float side = r * 1.2f;
            const sgl::Vec2f size{side, side};
            const sgl::Vec2f pos{center.x - size.x * 0.5f, center.y - size.y * 0.5f};
            if(const auto spr = sprites->sprite(spriteName(node.kind), pos, size, spriteTint(node.kind, theme)))
            {
                queue.push(sgl::Layer::World, 1500.f + static_cast<float>(i), *spr);
            }
        }

        queue.push(sgl::Layer::Hud, 2000.f + static_cast<float>(i),
                   sgl::TextCmd{
                       .font = font,
                       .text = shortLabel(node.kind),
                       .size = 11,
                       .position = {center.x, center.y + r + 2.f},
                       .color = sgl::Color{240, 240, 245},
                       .anchor = sgl::Anchor::Center,
                   });
    }
}

} // namespace sgl::syndicate
