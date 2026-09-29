#pragma once

#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/SpriteBank.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <syndicate/sim/Pathing.hpp>
#include <vector>

namespace sgl::syndicate
{

/// Emits one `CircleCmd` per node and one `LineCmd` per unique unordered edge pair on `Layer::World`.
/// Optional short labels (40 `TextCmd`) and atlas sprites when `sprites` is non-null.
void renderBoard(sgl::RenderQueue& queue,
                 const BoardGraph& board,
                 const Ownership& ownership,
                 const LayoutView& layout,
                 const Theme& theme,
                 const std::vector<MoveOption>& highlighted,
                 sgl::FontId font = {},
                 const SpriteBank* sprites = nullptr);

} // namespace sgl::syndicate
