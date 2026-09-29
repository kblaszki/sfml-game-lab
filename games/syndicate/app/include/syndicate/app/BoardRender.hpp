#pragma once

#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <syndicate/sim/Pathing.hpp>
#include <vector>

namespace sgl::syndicate
{

/// Emits one `CircleCmd` per node and one `LineCmd` per unique unordered edge pair on `Layer::World`.
void renderBoard(sgl::RenderQueue& queue,
                 const BoardGraph& board,
                 const Ownership& ownership,
                 const LayoutView& layout,
                 const Theme& theme,
                 const std::vector<MoveOption>& highlighted);

} // namespace sgl::syndicate
