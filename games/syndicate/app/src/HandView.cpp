#include <syndicate/app/HandView.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sgl/anim/Easing.hpp>
#include <sgl/core/Color.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <syndicate/app/Describe.hpp>
#include <utility>

namespace sgl::syndicate
{
namespace
{

constexpr float kCardW{96.f};
constexpr float kCardH{134.f};
constexpr float kGap{8.f};
constexpr float kRowStride{104.f};
constexpr float kPanelLeft{16.f};
constexpr float kPanelRight{340.f};
constexpr float kColX{24.f};
constexpr float kColY0{420.f};
constexpr float kRowY{480.f};
constexpr float kAnimDur{0.35f};
constexpr float kEnterFromX{-120.f};
constexpr float kLeaveToX{400.f};
constexpr float kCorner{8.f};

[[nodiscard]] sgl::Color withAlpha(sgl::Color c, float alpha)
{
    const float a = std::clamp(alpha, 0.f, 1.f);
    return sgl::Color{c.r, c.g, c.b, static_cast<std::uint8_t>(std::lround(static_cast<float>(c.a) * a))};
}

} // namespace

bool HandView::useRowLayout() noexcept
{
    return static_cast<float>(Hand::kMax) * kRowStride < (kPanelRight - kPanelLeft);
}

sgl::Vec2f HandView::slotPos(std::size_t index, std::size_t count) noexcept
{
    if(useRowLayout())
    {
        const float total = static_cast<float>(count) * kCardW + static_cast<float>(count > 0 ? count - 1 : 0) * kGap;
        const float x0 = kPanelLeft + std::max(0.f, (kPanelRight - kPanelLeft - total) * 0.5f);
        return {x0 + static_cast<float>(index) * kRowStride, kRowY};
    }
    return {kColX, kColY0 + static_cast<float>(index) * (kCardH + kGap)};
}

void HandView::reconcile(const Hand& hand)
{
    const auto cards = hand.cards();
    std::vector<bool> slotMatched(slots_.size(), false);
    std::vector<std::size_t> cardToSlot(cards.size(), static_cast<std::size_t>(-1));

    for(std::size_t ci = 0; ci < cards.size(); ++ci)
    {
        for(std::size_t si = 0; si < slots_.size(); ++si)
        {
            if(slots_[si].leaving || slotMatched[si])
            {
                continue;
            }
            if(slots_[si].kind == cards[ci])
            {
                slotMatched[si] = true;
                cardToSlot[ci] = si;
                break;
            }
        }
    }

    for(std::size_t si = 0; si < slots_.size(); ++si)
    {
        if(slots_[si].leaving || slotMatched[si])
        {
            continue;
        }
        Slot& slot = slots_[si];
        slot.leaving = true;
        slot.leaveElapsed = sgl::Seconds{0.f};
        const sgl::Vec2f cur = slot.pos.value();
        slot.pos.start(cur, sgl::Vec2f{kLeaveToX, cur.y}, sgl::Seconds{kAnimDur}, sgl::easeOutCubic);
    }

    handOrder_.clear();
    handOrder_.reserve(cards.size());

    for(std::size_t ci = 0; ci < cards.size(); ++ci)
    {
        const sgl::Vec2f dest = slotPos(ci, cards.size());
        const sgl::Rect<float> rect{.pos = dest, .size = {kCardW, kCardH}};

        if(cardToSlot[ci] == static_cast<std::size_t>(-1))
        {
            Slot slot{};
            slot.kind = cards[ci];
            slot.slotRect = rect;
            slot.alpha = 1.f;
            slot.leaving = false;
            slot.pos.start(sgl::Vec2f{kEnterFromX, dest.y}, dest, sgl::Seconds{kAnimDur}, sgl::easeOutBack);
            slots_.push_back(std::move(slot));
            handOrder_.push_back(slots_.size() - 1);
        }
        else
        {
            Slot& slot = slots_[cardToSlot[ci]];
            if(slot.slotRect.pos.x != dest.x || slot.slotRect.pos.y != dest.y)
            {
                const sgl::Vec2f cur = slot.pos.value();
                slot.pos.start(cur, dest, sgl::Seconds{kAnimDur}, sgl::easeOutBack);
            }
            slot.slotRect = rect;
            handOrder_.push_back(cardToSlot[ci]);
        }
    }
}

void HandView::rebuildHotspots()
{
    hotspots_.clear();
    hotspots_.reserve(handOrder_.size());
    for(const std::size_t si: handOrder_)
    {
        if(si >= slots_.size() || slots_[si].leaving)
        {
            continue;
        }
        const Slot& slot = slots_[si];
        const std::string name{describeCard(slot.kind)};
        hotspots_.push_back(sgl::Hotspot{
            .area = slot.slotRect,
            .title = name,
            .body = name,
        });
    }
}

void HandView::sync(const Hand& hand, sgl::Seconds dt)
{
    reconcile(hand);

    for(Slot& slot: slots_)
    {
        slot.pos.update(dt);
        if(slot.leaving)
        {
            slot.leaveElapsed += dt;
            slot.alpha = std::clamp(1.f - slot.leaveElapsed.count() / kAnimDur, 0.f, 1.f);
        }
    }

    std::vector<std::size_t> remap(slots_.size(), static_cast<std::size_t>(-1));
    std::size_t write = 0;
    for(std::size_t si = 0; si < slots_.size(); ++si)
    {
        if(slots_[si].leaving && slots_[si].leaveElapsed.count() >= kAnimDur)
        {
            continue;
        }
        if(write != si)
        {
            slots_[write] = std::move(slots_[si]);
        }
        remap[si] = write;
        ++write;
    }
    slots_.resize(write);

    for(std::size_t& idx: handOrder_)
    {
        if(idx < remap.size())
        {
            idx = remap[idx];
        }
    }
    std::erase_if(handOrder_, [](std::size_t idx) { return idx == static_cast<std::size_t>(-1); });

    rebuildHotspots();
}

void HandView::render(sgl::RenderQueue& queue, sgl::FontId font, const Theme& theme) const
{
    float depth = 0.f;
    for(const Slot& slot: slots_)
    {
        const sgl::Vec2f pos = slot.pos.value();
        const sgl::Rect<float> drawn{.pos = pos, .size = {kCardW, kCardH}};
        const sgl::Color fill =
            withAlpha(sgl::Color{static_cast<std::uint8_t>(theme.edge.r / 2),
                                 static_cast<std::uint8_t>(theme.edge.g / 2),
                                 static_cast<std::uint8_t>(theme.edge.b / 2),
                                 230},
                      slot.alpha);
        const sgl::Color textColor = withAlpha(sgl::Color{235, 238, 245}, slot.alpha);

        queue.push(sgl::Layer::Hud, depth++, sgl::roundedRect(drawn, kCorner, fill));
        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::TextCmd{
                       .font = font,
                       .text = std::string{describeCard(slot.kind)},
                       .size = 12,
                       .position = {pos.x + kCardW * 0.5f, pos.y + kCardH * 0.5f},
                       .color = textColor,
                       .anchor = sgl::Anchor::Center,
                   });
    }
}

std::optional<std::size_t> HandView::hit(sgl::Vec2f pointer) const
{
    for(std::size_t i = 0; i < handOrder_.size(); ++i)
    {
        const std::size_t si = handOrder_[i];
        if(si >= slots_.size() || slots_[si].leaving)
        {
            continue;
        }
        if(slots_[si].slotRect.contains(pointer))
        {
            return i;
        }
    }
    return std::nullopt;
}

std::span<const sgl::Hotspot> HandView::hotspots() const
{
    return hotspots_;
}

} // namespace sgl::syndicate
