#include <algorithm>
#include <cstddef>
#include <optional>
#include <syndicate/sim/Pathing.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] std::optional<NodeId> findPayday(const BoardGraph& board)
{
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const auto id = NodeId{static_cast<std::uint16_t>(i)};
        if(std::holds_alternative<Payday>(board.node(id).kind))
        {
            return id;
        }
    }
    return std::nullopt;
}

[[nodiscard]] bool pathLess(const std::vector<NodeId>& a, const std::vector<NodeId>& b)
{
    return a < b;
}

void dfs(const BoardGraph& board,
         NodeId payday,
         NodeId current,
         std::optional<NodeId> prev,
         int remaining,
         std::vector<NodeId>& path,
         bool passedPayday,
         std::vector<MoveOption>& out)
{
    if(remaining == 0)
    {
        out.push_back(MoveOption{current, path, passedPayday});
        return;
    }

    for(const auto next: board.successors(current))
    {
        if(prev.has_value() && next == *prev)
        {
            continue;
        }
        const bool nextPassed = passedPayday || (next == payday);
        path.push_back(next);
        dfs(board, payday, next, current, remaining - 1, path, nextPassed, out);
        path.pop_back();
    }
}

} // namespace

std::vector<MoveOption> moveOptions(const BoardGraph& board, NodeId from, std::optional<NodeId> cameFrom, int steps)
{
    if(steps < 0)
    {
        return {};
    }
    if(steps > 12)
    {
        steps = 12;
    }

    const auto paydayOpt = findPayday(board);
    if(!paydayOpt.has_value())
    {
        return {};
    }
    const auto payday = *paydayOpt;

    if(steps == 0)
    {
        return {MoveOption{from, std::vector<NodeId>{from}, false}};
    }

    std::vector<MoveOption> walks;
    std::vector<NodeId> path{from};
    dfs(board, payday, from, cameFrom, steps, path, false, walks);

    // Dedupe by (destination, passesPayday); keep lex-smaller path.
    std::vector<MoveOption> unique;
    unique.reserve(walks.size());
    for(auto& opt: walks)
    {
        const auto it = std::find_if(unique.begin(), unique.end(), [&](const MoveOption& existing) {
            return existing.destination == opt.destination && existing.passesPayday == opt.passesPayday;
        });
        if(it == unique.end())
        {
            unique.push_back(std::move(opt));
        }
        else if(pathLess(opt.path, it->path))
        {
            *it = std::move(opt);
        }
    }

    std::sort(unique.begin(), unique.end(), [](const MoveOption& a, const MoveOption& b) {
        if(a.destination != b.destination)
        {
            return a.destination < b.destination;
        }
        if(a.passesPayday != b.passesPayday)
        {
            return a.passesPayday < b.passesPayday; // false before true
        }
        return a.path < b.path;
    });

    return unique;
}

} // namespace sgl::syndicate
