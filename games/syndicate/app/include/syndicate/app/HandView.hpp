#pragma once

#include <cstddef>
#include <optional>
#include <sgl/anim/Tween.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/TooltipSystem.hpp>
#include <span>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/Cards.hpp>
#include <vector>

namespace sgl::syndicate
{

class HandView
{
public:
    void setPlayableKinds(std::span<const CardKind> kinds);
    void sync(const Hand& hand, sgl::Seconds dt);
    void render(sgl::RenderQueue& queue, sgl::FontId font, const Theme& theme) const;
    [[nodiscard]] std::optional<std::size_t> hit(sgl::Vec2f pointer) const;
    [[nodiscard]] std::span<const sgl::Hotspot> hotspots() const;

private:
    struct Slot
    {
        CardKind kind{};
        sgl::Rect<float> slotRect{};
        sgl::Tween<sgl::Vec2f> pos{};
        float alpha{1.f};
        bool leaving{false};
        bool playable{false};
        sgl::Seconds leaveElapsed{};
    };

    void reconcile(const Hand& hand);
    void rebuildHotspots();
    [[nodiscard]] static bool useRowLayout() noexcept;
    [[nodiscard]] static sgl::Vec2f slotPos(std::size_t index, std::size_t count) noexcept;

    std::vector<Slot> slots_;
    std::vector<std::size_t> handOrder_;
    std::vector<sgl::Hotspot> hotspots_;
    std::vector<CardKind> playableKinds_;
};

} // namespace sgl::syndicate
