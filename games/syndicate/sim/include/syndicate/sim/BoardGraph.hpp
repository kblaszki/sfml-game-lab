#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>
#include <vector>

namespace sgl::syndicate
{

struct RingSlot
{
    std::uint8_t ring;
    std::uint8_t slot;
};

struct Payday
{
};

struct Exchange
{
};

struct Hub
{
};

struct AuditTile
{
    Percent tax;
};

struct Grant
{
    Money amount;
};

struct SectorTile
{
    Sector sector;
    Money basePrice;
    Money baseFee;
};

using TileKind = std::variant<SectorTile, Payday, Exchange, Hub, AuditTile, Grant>;

struct TileNode
{
    NodeId id;
    TileKind kind;
    RingSlot at;
};

struct Edge
{
    NodeId from;
    NodeId to;
};

enum class BoardError
{
    DuplicateId,
    NonDenseIds,
    DanglingEdge,
    DeadEnd,
    NotStronglyConnected,
    PaydayCount,
};

class BoardGraph
{
public:
    [[nodiscard]] static std::expected<BoardGraph, BoardError> build(std::vector<TileNode> nodes,
                                                                     std::span<const Edge> edges);

    [[nodiscard]] std::span<const NodeId> successors(NodeId id) const;
    [[nodiscard]] const TileNode& node(NodeId id) const;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    BoardGraph(std::vector<TileNode> nodes, std::vector<std::uint32_t> offsets, std::vector<NodeId> targets);

    std::vector<TileNode> nodes_;
    std::vector<std::uint32_t> offsets_;
    std::vector<NodeId> targets_;
};

} // namespace sgl::syndicate
