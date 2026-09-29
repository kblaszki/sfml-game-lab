#include <cstddef>
#include <cstdint>
#include <syndicate/sim/BoardGraph.hpp>
#include <vector>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] bool
isStronglyConnected(std::size_t n, std::span<const std::uint32_t> offsets, std::span<const NodeId> targets)
{
    if(n == 0)
    {
        return true;
    }

    auto dfsReach = [&](std::size_t start, auto successorFn) {
        std::vector<char> seen(n, 0);
        std::vector<std::size_t> stack;
        stack.push_back(start);
        seen[start] = 1;
        std::size_t count = 0;
        while(!stack.empty())
        {
            const auto u = stack.back();
            stack.pop_back();
            ++count;
            successorFn(u, [&](std::size_t v) {
                if(!seen[v])
                {
                    seen[v] = 1;
                    stack.push_back(v);
                }
            });
        }
        return count == n;
    };

    const bool forwardOk = dfsReach(0, [&](std::size_t u, auto visit) {
        for(auto i = offsets[u]; i < offsets[u + 1]; ++i)
        {
            visit(static_cast<std::size_t>(targets[i]));
        }
    });
    if(!forwardOk)
    {
        return false;
    }

    std::vector<std::vector<std::size_t>> rev(n);
    for(std::size_t u = 0; u < n; ++u)
    {
        for(auto i = offsets[u]; i < offsets[u + 1]; ++i)
        {
            rev[static_cast<std::size_t>(targets[i])].push_back(u);
        }
    }

    return dfsReach(0, [&](std::size_t u, auto visit) {
        for(const auto v: rev[u])
        {
            visit(v);
        }
    });
}

} // namespace

BoardGraph::BoardGraph(std::vector<TileNode> nodes, std::vector<std::uint32_t> offsets, std::vector<NodeId> targets)
    : nodes_(std::move(nodes))
    , offsets_(std::move(offsets))
    , targets_(std::move(targets))
{
}

std::expected<BoardGraph, BoardError> BoardGraph::build(std::vector<TileNode> nodes, std::span<const Edge> edges)
{
    const auto n = nodes.size();
    std::vector<char> seen(n, 0);
    for(const auto& node: nodes)
    {
        const auto id = static_cast<std::size_t>(node.id);
        if(id >= n)
        {
            return std::unexpected(BoardError::NonDenseIds);
        }
        if(seen[id])
        {
            return std::unexpected(BoardError::DuplicateId);
        }
        seen[id] = 1;
    }
    for(std::size_t i = 0; i < n; ++i)
    {
        if(!seen[i])
        {
            return std::unexpected(BoardError::NonDenseIds);
        }
    }

    std::vector<TileNode> ordered(n);
    for(auto& node: nodes)
    {
        ordered[static_cast<std::size_t>(node.id)] = node;
    }

    std::vector<std::vector<NodeId>> adj(n);
    for(const auto& e: edges)
    {
        const auto from = static_cast<std::size_t>(e.from);
        const auto to = static_cast<std::size_t>(e.to);
        if(from >= n || to >= n)
        {
            return std::unexpected(BoardError::DanglingEdge);
        }
        adj[from].push_back(e.to);
    }

    for(std::size_t i = 0; i < n; ++i)
    {
        if(adj[i].empty())
        {
            return std::unexpected(BoardError::DeadEnd);
        }
    }

    std::vector<std::uint32_t> offsets(n + 1, 0);
    std::size_t edgeCount = 0;
    for(std::size_t i = 0; i < n; ++i)
    {
        edgeCount += adj[i].size();
        offsets[i + 1] = static_cast<std::uint32_t>(edgeCount);
    }
    std::vector<NodeId> targets;
    targets.reserve(edgeCount);
    for(std::size_t i = 0; i < n; ++i)
    {
        for(const auto t: adj[i])
        {
            targets.push_back(t);
        }
    }

    if(!isStronglyConnected(n, offsets, targets))
    {
        return std::unexpected(BoardError::NotStronglyConnected);
    }

    std::size_t paydayCount = 0;
    for(const auto& node: ordered)
    {
        if(std::holds_alternative<Payday>(node.kind))
        {
            ++paydayCount;
        }
    }
    if(paydayCount != 1)
    {
        return std::unexpected(BoardError::PaydayCount);
    }

    return BoardGraph{std::move(ordered), std::move(offsets), std::move(targets)};
}

std::span<const NodeId> BoardGraph::successors(NodeId id) const
{
    const auto i = static_cast<std::size_t>(id);
    const auto begin = offsets_[i];
    const auto end = offsets_[i + 1];
    return std::span<const NodeId>{targets_.data() + begin, end - begin};
}

const TileNode& BoardGraph::node(NodeId id) const
{
    return nodes_[static_cast<std::size_t>(id)];
}

std::size_t BoardGraph::size() const noexcept
{
    return nodes_.size();
}

} // namespace sgl::syndicate
