#pragma once

#include <optional>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ids.hpp>
#include <vector>

namespace sgl::syndicate
{

struct MoveOption
{
    NodeId destination;
    std::vector<NodeId> path;
    bool passesPayday;
};

[[nodiscard]] std::vector<MoveOption>
moveOptions(const BoardGraph& board, NodeId from, std::optional<NodeId> cameFrom, int steps);

} // namespace sgl::syndicate
