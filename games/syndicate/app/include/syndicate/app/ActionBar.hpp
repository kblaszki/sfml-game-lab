#pragma once

#include <cstddef>
#include <optional>
#include <sgl/input/InputState.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/HoverAnim.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <sgl/ui/TooltipSystem.hpp>
#include <span>
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
    void rebuild(const Match& match, PlayerId actor, std::optional<NodeId> selectedTile, bool offerTrade = false);

    [[nodiscard]] std::optional<Command> update(const sgl::InputState& input, const SyndicateActions& actions);

    /// True once after the app-only Trade slot is activated by Num or click. Clears the flag.
    [[nodiscard]] bool consumeTradeRequest();

    void render(sgl::RenderQueue& queue, const sgl::MenuStyle& style, const sgl::HoverAnim& hover) const;

    [[nodiscard]] std::span<const sgl::Hotspot> hotspots() const;

    [[nodiscard]] std::size_t size() const noexcept;

private:
    struct Entry
    {
        std::optional<Command> command{};
        bool trade{false};
        sgl::Rect<float> rect{};
        std::string label{};
        sgl::Hotspot hotspot{};
    };

    std::vector<Entry> entries_;
    std::vector<sgl::Hotspot> hotspots_;
    bool tradeRequested_{false};
};

} // namespace sgl::syndicate
