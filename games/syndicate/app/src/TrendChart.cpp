#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <syndicate/app/Describe.hpp>
#include <syndicate/app/TrendChart.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr std::array<std::string_view, 6> kShortNames{"AI", "CLN", "BIO", "CYB", "LOG", "FIN"};

[[nodiscard]] Sector sectorAt(std::size_t i) noexcept
{
    return static_cast<Sector>(static_cast<std::uint8_t>(i));
}

[[nodiscard]] sgl::Rect<float> rowRect(sgl::Rect<float> area, std::size_t row) noexcept
{
    const float h = area.size.y / 6.f;
    return sgl::Rect<float>{.pos = {area.pos.x, area.pos.y + static_cast<float>(row) * h}, .size = {area.size.x, h}};
}

[[nodiscard]] float clampX(float x, const sgl::Rect<float>& area) noexcept
{
    return std::clamp(x, area.pos.x, area.pos.x + area.size.x);
}

[[nodiscard]] float clampY(float y, const sgl::Rect<float>& area) noexcept
{
    return std::clamp(y, area.pos.y, area.pos.y + area.size.y);
}

[[nodiscard]] std::string formatLabel(std::size_t sectorIdx, std::span<const Percent> series)
{
    std::ostringstream oss;
    oss << kShortNames[sectorIdx];
    if(series.empty())
    {
        return oss.str();
    }
    oss << ' ' << series.back().v;
    if(series.size() >= 2)
    {
        const int delta = series.back().v - series[series.size() - 2].v;
        oss << ' ' << (delta >= 0 ? '+' : '-') << std::abs(delta);
    }
    return oss.str();
}

[[nodiscard]] std::string formatHotspotBody(Sector sector, std::span<const Percent> series)
{
    std::ostringstream oss;
    oss << sectorName(sector);
    if(series.empty())
    {
        oss << " index —";
        return oss.str();
    }
    const Percent latest = series.back();
    oss << " index " << latest.v;
    if(series.size() >= 2)
    {
        const int delta = latest.v - series[series.size() - 2].v;
        oss << " (" << (delta >= 0 ? '+' : '-') << std::abs(delta) << ')';
    }
    const double mult = static_cast<double>(latest.v) / 100.0;
    oss << ", share price x" << std::fixed << std::setprecision(1) << mult;
    return oss.str();
}

} // namespace

void renderTrendChart(sgl::RenderQueue& queue,
                      const MarketHistory& history,
                      sgl::Rect<float> area,
                      const Theme& theme,
                      sgl::FontId font)
{
    float depth = 0.f;
    queue.push(sgl::Layer::Hud, depth++,
               sgl::verticalGradient(area, sgl::Color{36, 42, 58, 220}, sgl::Color{22, 26, 38, 230}));

    for(std::size_t i = 0; i < 6; ++i)
    {
        const sgl::Rect<float> row = rowRect(area, i);
        const auto series = history.series(sectorAt(i));
        const std::string label = formatLabel(i, series);

        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::TextCmd{
                       .font = font,
                       .text = label,
                       .size = 12,
                       .position = sgl::Vec2f{row.pos.x + 6.f, row.pos.y + 4.f},
                       .color = theme.sector[i],
                   });

        if(series.size() < 2)
        {
            continue;
        }

        std::int32_t lo = series.front().v;
        std::int32_t hi = series.front().v;
        for(const Percent& p: series)
        {
            lo = std::min(lo, p.v);
            hi = std::max(hi, p.v);
        }
        const float range = static_cast<float>(std::max(hi - lo, 1));

        constexpr float kLabelReserve = 22.f;
        const float plotTop = row.pos.y + kLabelReserve;
        const float plotBottom = row.pos.y + row.size.y - 4.f;
        const float plotH = std::max(plotBottom - plotTop, 1.f);
        const float plotLeft = row.pos.x + 6.f;
        const float plotRight = row.pos.x + row.size.x - 6.f;
        const float plotW = std::max(plotRight - plotLeft, 1.f);

        std::vector<sgl::Vec2f> points;
        points.reserve(series.size());
        for(std::size_t s = 0; s < series.size(); ++s)
        {
            const float t = static_cast<float>(s) / static_cast<float>(series.size() - 1);
            const float x = clampX(plotLeft + t * plotW, area);
            const float norm = static_cast<float>(series[s].v - lo) / range;
            const float y = clampY(plotBottom - norm * plotH, area);
            points.push_back(sgl::Vec2f{x, y});
        }
        queue.push(sgl::Layer::Hud, depth++, sgl::polyline(points, 2.f, theme.sector[i]));
    }
}

std::vector<sgl::Hotspot> chartHotspots(const MarketHistory& history, sgl::Rect<float> area)
{
    std::vector<sgl::Hotspot> spots;
    spots.reserve(6);
    for(std::size_t i = 0; i < 6; ++i)
    {
        const Sector sector = sectorAt(i);
        const auto series = history.series(sector);
        spots.push_back(sgl::Hotspot{
            .area = rowRect(area, i),
            .title = std::string{sectorName(sector)},
            .body = formatHotspotBody(sector, series),
        });
    }
    return spots;
}

} // namespace sgl::syndicate
