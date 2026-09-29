#pragma once

#include <cstddef>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/TooltipSystem.hpp>
#include <string>
#include <syndicate/app/MarketHistory.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Match.hpp>
#include <vector>

namespace sgl::syndicate
{

struct HudNote
{
    std::string text;
    float ttl{};
};

[[nodiscard]] inline sgl::Rect<float> playerPanelRect(int seatIndex) noexcept
{
    return sgl::Rect<float>{.pos = {16.f, 48.f + static_cast<float>(seatIndex) * 36.f}, .size = {300.f, 32.f}};
}

[[nodiscard]] inline sgl::Rect<float> marketChartRect() noexcept
{
    return sgl::Rect<float>{.pos = {940.f, 16.f}, .size = {324.f, 284.f}};
}

class HudModel
{
public:
    void pushNote(std::string text, float ttl);
    void recordMarket(const Market& market);
    void update(sgl::Seconds dt);
    void render(sgl::RenderQueue& queue, const Match& match, sgl::FontId font, const Theme& theme) const;

    [[nodiscard]] std::string phasePrompt(const Phase& phase) const;
    [[nodiscard]] std::size_t noteCount() const noexcept;
    [[nodiscard]] std::vector<sgl::Hotspot> chartHotspots() const;

private:
    std::vector<HudNote> notes_;
    MarketHistory history_{};
};

} // namespace sgl::syndicate
