#include <syndicate/app/ActivityLog.hpp>

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <sgl/ui/TextLayout.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr float kCorner{6.f};
constexpr float kStripeW{4.f};
constexpr float kPad{8.f};
constexpr unsigned kTextSize{14};
constexpr float kLineH{18.f};

} // namespace

void ActivityLog::push(int round, std::optional<PlayerId> player, std::string text)
{
    entries_.push_back(LogEntry{.round = round, .player = player, .text = std::move(text)});
    if(entries_.size() > kCapacity)
    {
        entries_.erase(entries_.begin(),
                       entries_.begin() + static_cast<std::ptrdiff_t>(entries_.size() - kCapacity));
    }
}

std::span<const LogEntry> ActivityLog::visible(std::size_t n) const
{
    if(entries_.empty() || n == 0)
    {
        return {};
    }
    const std::size_t count = std::min(n, entries_.size());
    const std::size_t start = entries_.size() - count;
    return std::span<const LogEntry>{entries_.data() + start, count};
}

void ActivityLog::render(sgl::RenderQueue& queue, sgl::Rect<float> area, const Theme& theme, sgl::FontId font) const
{
    float depth = 80.f;
    queue.push(sgl::Layer::Hud, depth++, sgl::roundedRect(area, kCorner, sgl::Color{36, 42, 58, 210}));
    queue.push(sgl::Layer::Hud,
               depth++,
               sgl::roundedRectOutline(area, kCorner, 1.f, sgl::Color{80, 90, 120, 200}));

    const float textLeft = area.pos.x + kPad + kStripeW + 4.f;
    const float maxWidth = area.pos.x + area.size.x - kPad - textLeft;
    const float bottom = area.pos.y + area.size.y - kPad;

    // Estimate how many entries fit from the bottom, then draw oldest→newest top→bottom.
    std::vector<std::pair<const LogEntry*, std::vector<std::string>>> blocks;
    float used = 0.f;
    for(std::size_t i = entries_.size(); i > 0; --i)
    {
        const LogEntry& entry = entries_[i - 1];
        auto lines = sgl::wrap(entry.text, kTextSize, maxWidth);
        if(lines.empty())
        {
            lines.push_back({});
        }
        const float blockH = static_cast<float>(lines.size()) * kLineH;
        if(used + blockH > area.size.y - 2.f * kPad && !blocks.empty())
        {
            break;
        }
        used += blockH;
        blocks.push_back({&entry, std::move(lines)});
    }
    std::ranges::reverse(blocks);

    float y = bottom - used;
    for(const auto& [entry, lines]: blocks)
    {
        const float blockH = static_cast<float>(lines.size()) * kLineH;
        if(entry->player.has_value())
        {
            const std::size_t pi = static_cast<std::size_t>(std::to_underlying(*entry->player));
            if(pi < 4)
            {
                const sgl::Rect<float> stripe{{area.pos.x + kPad, y}, {kStripeW, blockH}};
                queue.push(sgl::Layer::Hud, depth++, sgl::roundedRect(stripe, 1.f, theme.pawn[pi]));
            }
        }
        for(const std::string& line: lines)
        {
            queue.push(sgl::Layer::Hud,
                       depth++,
                       sgl::TextCmd{
                           .font = font,
                           .text = line,
                           .size = kTextSize,
                           .position = sgl::Vec2f{textLeft, y},
                           .color = sgl::Color{220, 225, 235},
                       });
            y += kLineH;
        }
    }
}

} // namespace sgl::syndicate
