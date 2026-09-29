#include <array>
#include <span>
#include <stdexcept>
#include <syndicate/sim/BoardLayouts.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr Money kOuterPrice{100};
constexpr Money kOuterFee{10};
constexpr Money kInnerPrice{150};
constexpr Money kInnerFee{15};
constexpr Money kCorePrice{300};
constexpr Money kCoreFee{30};
constexpr Percent kAuditTax{10};
constexpr Money kGrantAmount{100};

[[nodiscard]] TileNode
sectorNode(NodeId id, std::uint8_t ring, std::uint8_t slot, Sector sector, Money price, Money fee)
{
    return TileNode{id, SectorTile{sector, price, fee}, RingSlot{ring, slot}};
}

[[nodiscard]] BoardGraph mustBuild(std::vector<TileNode> nodes, std::span<const Edge> edges)
{
    auto built = BoardGraph::build(std::move(nodes), edges);
    if(!built)
    {
        throw std::logic_error("standardBoard layout failed BoardGraph::build");
    }
    return std::move(*built);
}

} // namespace

BoardGraph standardBoard()
{
    std::vector<TileNode> nodes;
    nodes.reserve(40);

    // Outer ring 0..23 (CW). Slot 0 = Payday.
    nodes.push_back(TileNode{NodeId{0}, Payday{}, RingSlot{0, 0}});
    nodes.push_back(sectorNode(NodeId{1}, 0, 1, Sector::AI, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{2}, 0, 2, Sector::CleanEnergy, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{3}, 0, 3, Sector::Biotech, kOuterPrice, kOuterFee));
    nodes.push_back(TileNode{NodeId{4}, Exchange{}, RingSlot{0, 4}});
    nodes.push_back(sectorNode(NodeId{5}, 0, 5, Sector::CyberSec, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{6}, 0, 6, Sector::Logistics, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{7}, 0, 7, Sector::Fintech, kOuterPrice, kOuterFee));
    nodes.push_back(TileNode{NodeId{8}, Hub{}, RingSlot{0, 8}});
    nodes.push_back(sectorNode(NodeId{9}, 0, 9, Sector::AI, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{10}, 0, 10, Sector::CleanEnergy, kOuterPrice, kOuterFee));
    nodes.push_back(TileNode{NodeId{11}, AuditTile{kAuditTax}, RingSlot{0, 11}});
    nodes.push_back(sectorNode(NodeId{12}, 0, 12, Sector::Biotech, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{13}, 0, 13, Sector::CyberSec, kOuterPrice, kOuterFee));
    nodes.push_back(TileNode{NodeId{14}, Grant{kGrantAmount}, RingSlot{0, 14}});
    nodes.push_back(sectorNode(NodeId{15}, 0, 15, Sector::Logistics, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{16}, 0, 16, Sector::Fintech, kOuterPrice, kOuterFee));
    nodes.push_back(TileNode{NodeId{17}, Exchange{}, RingSlot{0, 17}});
    nodes.push_back(sectorNode(NodeId{18}, 0, 18, Sector::AI, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{19}, 0, 19, Sector::CleanEnergy, kOuterPrice, kOuterFee));
    nodes.push_back(TileNode{NodeId{20}, Hub{}, RingSlot{0, 20}});
    nodes.push_back(sectorNode(NodeId{21}, 0, 21, Sector::Biotech, kOuterPrice, kOuterFee));
    nodes.push_back(sectorNode(NodeId{22}, 0, 22, Sector::CyberSec, kOuterPrice, kOuterFee));
    nodes.push_back(TileNode{NodeId{23}, Grant{kGrantAmount}, RingSlot{0, 23}});

    // Inner ring 24..35 (CCW along decreasing slot).
    nodes.push_back(sectorNode(NodeId{24}, 1, 0, Sector::Logistics, kInnerPrice, kInnerFee));
    nodes.push_back(sectorNode(NodeId{25}, 1, 1, Sector::Fintech, kInnerPrice, kInnerFee));
    nodes.push_back(TileNode{NodeId{26}, Exchange{}, RingSlot{1, 2}});
    nodes.push_back(sectorNode(NodeId{27}, 1, 3, Sector::AI, kInnerPrice, kInnerFee));
    nodes.push_back(TileNode{NodeId{28}, Hub{}, RingSlot{1, 4}});
    nodes.push_back(sectorNode(NodeId{29}, 1, 5, Sector::CleanEnergy, kInnerPrice, kInnerFee));
    nodes.push_back(sectorNode(NodeId{30}, 1, 6, Sector::Biotech, kInnerPrice, kInnerFee));
    nodes.push_back(TileNode{NodeId{31}, AuditTile{kAuditTax}, RingSlot{1, 7}});
    nodes.push_back(sectorNode(NodeId{32}, 1, 8, Sector::CyberSec, kInnerPrice, kInnerFee));
    nodes.push_back(sectorNode(NodeId{33}, 1, 9, Sector::Logistics, kInnerPrice, kInnerFee));
    nodes.push_back(TileNode{NodeId{34}, Grant{kGrantAmount}, RingSlot{1, 10}});
    nodes.push_back(sectorNode(NodeId{35}, 1, 11, Sector::Fintech, kInnerPrice, kInnerFee));

    // Core 36..39 premium sectors.
    nodes.push_back(sectorNode(NodeId{36}, 2, 0, Sector::AI, kCorePrice, kCoreFee));
    nodes.push_back(sectorNode(NodeId{37}, 2, 1, Sector::CleanEnergy, kCorePrice, kCoreFee));
    nodes.push_back(sectorNode(NodeId{38}, 2, 2, Sector::Biotech, kCorePrice, kCoreFee));
    nodes.push_back(sectorNode(NodeId{39}, 2, 3, Sector::CyberSec, kCorePrice, kCoreFee));

    std::vector<Edge> edges;
    edges.reserve(40 + 12); // rings + bridge pairs (2 each)

    for(std::uint16_t i = 0; i < 24; ++i)
    {
        edges.push_back(Edge{NodeId{i}, NodeId{static_cast<std::uint16_t>((i + 1) % 24)}});
    }
    for(std::uint8_t s = 0; s < 12; ++s)
    {
        const auto from = static_cast<std::uint16_t>(24 + s);
        const auto to = static_cast<std::uint16_t>(24 + ((s + 11) % 12));
        edges.push_back(Edge{NodeId{from}, NodeId{to}});
    }
    for(std::uint16_t i = 0; i < 4; ++i)
    {
        const auto from = static_cast<std::uint16_t>(36 + i);
        const auto to = static_cast<std::uint16_t>(36 + ((i + 1) % 4));
        edges.push_back(Edge{NodeId{from}, NodeId{to}});
    }

    constexpr std::array<std::pair<std::uint16_t, std::uint16_t>, 4> outerInner{{{0, 24}, {6, 27}, {12, 30}, {18, 33}}};
    for(const auto& [o, i]: outerInner)
    {
        edges.push_back(Edge{NodeId{o}, NodeId{i}});
        edges.push_back(Edge{NodeId{i}, NodeId{o}});
    }
    constexpr std::array<std::pair<std::uint16_t, std::uint16_t>, 2> innerCore{{{24, 36}, {30, 38}}};
    for(const auto& [i, c]: innerCore)
    {
        edges.push_back(Edge{NodeId{i}, NodeId{c}});
        edges.push_back(Edge{NodeId{c}, NodeId{i}});
    }

    return mustBuild(std::move(nodes), edges);
}

} // namespace sgl::syndicate
