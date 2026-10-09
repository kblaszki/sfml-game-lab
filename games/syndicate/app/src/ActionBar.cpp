#include <cstddef>
#include <sgl/core/Overloaded.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <sstream>
#include <string>
#include <syndicate/app/ActionBar.hpp>
#include <syndicate/app/Describe.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <utility>
#include <variant>

namespace sgl::syndicate
{
namespace
{

constexpr float kBarLeft{kActionBar.pos.x};
constexpr float kBarRight{kActionBar.pos.x + kActionBar.size.x};
constexpr float kBarTop{kActionBar.pos.y};
constexpr float kBarBottom{kActionBar.pos.y + kActionBar.size.y};
constexpr float kGap{8.f};
constexpr std::size_t kMaxSlots{9};

[[nodiscard]] std::optional<NodeId> commandTile(const Command& cmd)
{
    return std::visit(sgl::Overloaded{
                          [](const Upgrade& c) -> std::optional<NodeId> { return c.tile; },
                          [](const SellShares& c) -> std::optional<NodeId> { return c.tile; },
                          [](const ChooseDestination& c) -> std::optional<NodeId> { return c.destination; },
                          [](const PlayCard& c) -> std::optional<NodeId> {
                              if(const auto* tile = std::get_if<TileTarget>(&c.target))
                              {
                                  return tile->tile;
                              }
                              if(const auto* take = std::get_if<TakeoverTarget>(&c.target))
                              {
                                  return take->tile;
                              }
                              return std::nullopt;
                          },
                          [](const auto&) -> std::optional<NodeId> { return std::nullopt; },
                      },
                      cmd);
}

[[nodiscard]] bool isChooseDestination(const Command& cmd) noexcept
{
    return std::holds_alternative<ChooseDestination>(cmd);
}

[[nodiscard]] std::vector<Command>
filterCommands(std::vector<Command> legal, std::optional<NodeId> selectedTile, std::size_t maxSlots)
{
    std::erase_if(legal, isChooseDestination);
    if(legal.size() <= maxSlots)
    {
        return legal;
    }

    if(!selectedTile.has_value())
    {
        legal.resize(maxSlots);
        return legal;
    }

    std::vector<Command> preferred;
    std::vector<Command> nonTile;
    preferred.reserve(kMaxSlots);
    nonTile.reserve(legal.size());

    for(Command& cmd: legal)
    {
        const std::optional<NodeId> tile = commandTile(cmd);
        if(tile.has_value() && *tile == *selectedTile)
        {
            preferred.push_back(std::move(cmd));
        }
        else if(!tile.has_value())
        {
            nonTile.push_back(std::move(cmd));
        }
    }

    std::vector<Command> out;
    out.reserve(kMaxSlots);
    for(Command& cmd: preferred)
    {
        if(out.size() >= maxSlots)
        {
            break;
        }
        out.push_back(std::move(cmd));
    }
    for(Command& cmd: nonTile)
    {
        if(out.size() >= maxSlots)
        {
            break;
        }
        out.push_back(std::move(cmd));
    }
    return out;
}

[[nodiscard]] sgl::Rect<float> scaledAboutCenter(sgl::Rect<float> rect, float scale)
{
    if(scale == 1.f)
    {
        return rect;
    }
    const float cx = rect.pos.x + rect.size.x * 0.5f;
    const float cy = rect.pos.y + rect.size.y * 0.5f;
    const float w = rect.size.x * scale;
    const float h = rect.size.y * scale;
    return sgl::Rect<float>{.pos = {cx - w * 0.5f, cy - h * 0.5f}, .size = {w, h}};
}

} // namespace

void ActionBar::rebuild(const Match& match, PlayerId actor, std::optional<NodeId> selectedTile, bool offerTrade)
{
    entries_.clear();
    hotspots_.clear();
    tradeRequested_ = false;

    const std::size_t commandCap = offerTrade ? kMaxSlots - 1 : kMaxSlots;
    const std::vector<Command> filtered = filterCommands(match.legalCommands(actor), selectedTile, commandCap);
    const std::size_t slotCount = filtered.size() + (offerTrade ? 1u : 0u);
    if(slotCount == 0)
    {
        return;
    }

    const float totalWidth = kBarRight - kBarLeft;
    const float gaps = kGap * static_cast<float>(slotCount - 1);
    const float buttonWidth = (totalWidth - gaps) / static_cast<float>(slotCount);
    const float buttonHeight = kBarBottom - kBarTop;

    const GameState& state = match.state();
    entries_.reserve(slotCount);
    hotspots_.reserve(slotCount);

    auto pushEntry = [&](std::optional<Command> command,
                         bool trade,
                         std::size_t index,
                         const std::string& text,
                         const std::string& tooltip) {
        std::ostringstream shown;
        shown << '[' << (index + 1) << "] " << text;

        const sgl::Rect<float> rect{.pos = {kBarLeft + static_cast<float>(index) * (buttonWidth + kGap), kBarTop},
                                    .size = {buttonWidth, buttonHeight}};

        Entry entry{
            .command = std::move(command),
            .trade = trade,
            .rect = rect,
            .label = shown.str(),
            .hotspot =
                sgl::Hotspot{
                    .area = rect,
                    .title = shown.str(),
                    .body = tooltip,
                },
        };
        hotspots_.push_back(entry.hotspot);
        entries_.push_back(std::move(entry));
    };

    for(std::size_t i = 0; i < filtered.size(); ++i)
    {
        const Label label = describeCommand(state, actor, filtered[i]);
        pushEntry(filtered[i], false, i, label.text, label.tooltip);
    }
    if(offerTrade)
    {
        pushEntry(std::nullopt, true, filtered.size(), "Trade", "Propose a trade to another seat.");
    }
}

std::optional<Command> ActionBar::update(const sgl::InputState& input, const SyndicateActions& actions)
{
    tradeRequested_ = false;

    auto activated = [this](const Entry& entry) -> std::optional<Command> {
        if(entry.trade)
        {
            tradeRequested_ = true;
            return std::nullopt;
        }
        return entry.command;
    };

    for(std::size_t i = 0; i < entries_.size() && i < actions.slot.size(); ++i)
    {
        if(input.action(actions.slot[i]).pressed)
        {
            return activated(entries_[i]);
        }
    }

    if(input.pointerReleased())
    {
        if(const auto pointer = input.pointer())
        {
            for(const Entry& entry: entries_)
            {
                if(entry.rect.contains(*pointer))
                {
                    return activated(entry);
                }
            }
        }
    }

    return std::nullopt;
}

bool ActionBar::consumeTradeRequest()
{
    const bool requested = tradeRequested_;
    tradeRequested_ = false;
    return requested;
}

void ActionBar::render(sgl::RenderQueue& queue, const sgl::MenuStyle& style, const sgl::HoverAnim& hover) const
{
    if(entries_.empty())
    {
        return;
    }

    float depth = 0.f;
    for(std::size_t i = 0; i < entries_.size(); ++i)
    {
        const Entry& entry = entries_[i];
        const sgl::Rect<float> drawn = scaledAboutCenter(entry.rect, hover.scale(i));
        if(style.cornerRadius > 0.f)
        {
            queue.push(sgl::Layer::Hud, depth++, sgl::roundedRect(drawn, style.cornerRadius, style.idle));
        }
        else
        {
            queue.push(sgl::Layer::Hud,
                       depth++,
                       sgl::RectCmd{
                           .rect = drawn,
                           .fill = style.idle,
                       });
        }
        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::TextCmd{
                       .font = style.font,
                       .text = entry.label,
                       .size = style.textSize,
                       .position = {drawn.pos.x + drawn.size.x * 0.5f, drawn.pos.y + drawn.size.y * 0.5f},
                       .color = style.text,
                       .anchor = sgl::Anchor::Center,
                   });
    }
}

std::span<const sgl::Hotspot> ActionBar::hotspots() const
{
    return hotspots_;
}

std::size_t ActionBar::size() const noexcept
{
    return entries_.size();
}

} // namespace sgl::syndicate
