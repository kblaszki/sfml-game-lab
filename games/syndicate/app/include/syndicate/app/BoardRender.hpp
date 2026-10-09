#pragma once

#include <optional>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PathHighlighter.hpp>
#include <syndicate/app/SpriteBank.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <syndicate/sim/Pathing.hpp>
#include <vector>

namespace sgl::syndicate
{

/// Emits one `CircleCmd` per node and one `LineCmd` per unique unordered edge pair on `Layer::World`.
/// Optional short labels (40 `TextCmd`) and atlas sprites when `sprites` is non-null.
/// When `highlights` is non-null, destination nodes use a thicker outline.
/// `selectedTile` draws an extra selection ring; share pips show per-seat holdings.
void renderBoard(sgl::RenderQueue& queue,
                 const BoardGraph& board,
                 const Ownership& ownership,
                 const LayoutView& layout,
                 const Theme& theme,
                 const std::vector<MoveOption>& highlighted,
                 sgl::FontId font = {},
                 const SpriteBank* sprites = nullptr,
                 const HighlightModel* highlights = nullptr,
                 std::optional<NodeId> selectedTile = std::nullopt);

} // namespace sgl::syndicate
