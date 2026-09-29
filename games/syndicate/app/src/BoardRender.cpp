#include <cstddef>
#include <cstdint>
#include <set>
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
                return theme.hub;
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

} // namespace

void renderBoard(sgl::RenderQueue& queue,
                 const BoardGraph& board,
                 const Ownership& ownership,
                 const LayoutView& layout,
                 const Theme& theme,
                 const std::vector<MoveOption>& highlighted)
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
                   sgl::CircleCmd{.center = layout.position(node.at),
                                  .radius = r,
                                  .fill = tileFill(node.kind, theme),
                                  .outline = outline,
                                  .outlineThickness = static_cast<float>(hold.level) + 1.f,
                                  .points = 32});
    }
}

} // namespace sgl::syndicate
