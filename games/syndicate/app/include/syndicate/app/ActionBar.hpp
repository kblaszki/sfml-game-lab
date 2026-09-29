#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <sgl/input/InputState.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/HoverAnim.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <sgl/ui/TooltipSystem.hpp>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Match.hpp>
#include <vector>

namespace sgl::syndicate
{

class ActionBar
{
public:
    void rebuild(const Match& match, PlayerId actor, std::optional<NodeId> selectedTile);

    [[nodiscard]] std::optional<Command> update(const sgl::InputState& input, const SyndicateActions& actions);

    void render(sgl::RenderQueue& queue, const sgl::MenuStyle& style, const sgl::HoverAnim& hover) const;

    [[nodiscard]] std::span<const sgl::Hotspot> hotspots() const;

    [[nodiscard]] std::size_t size() const noexcept;

private:
    struct Entry
    {
        Command command{};
        sgl::Rect<float> rect{};
        std::string label{};
        sgl::Hotspot hotspot{};
    };

    std::vector<Entry> entries_;
    std::vector<sgl::Hotspot> hotspots_;
};

} // namespace sgl::syndicate
