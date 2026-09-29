#include <gtest/gtest.h>
#include <sgl/core/Rect.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <string>
#include <syndicate/app/MarketHistory.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/app/TrendChart.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>

using sgl::syndicate::Market;
using sgl::syndicate::MarketHistory;
using sgl::syndicate::Percent;
using sgl::syndicate::chartHotspots;
using sgl::syndicate::defaultTheme;
using sgl::syndicate::renderTrendChart;

namespace
{

[[nodiscard]] bool insideInclusive(sgl::Vec2f p, sgl::Rect<float> area, float tol = 1.f) noexcept
{
    return p.x >= area.pos.x - tol && p.x <= area.pos.x + area.size.x + tol && p.y >= area.pos.y - tol &&
           p.y <= area.pos.y + area.size.y + tol;
}

} // namespace

TEST(TrendChartTest, polylineVerticesStayInsideArea)
{
    MarketHistory history;
    Market a{};
    a.index = {Percent{100}, Percent{100}, Percent{100}, Percent{100}, Percent{100}, Percent{100}};
    Market b{};
    b.index = {Percent{108}, Percent{95}, Percent{112}, Percent{90}, Percent{105}, Percent{120}};
    history.record(a);
    history.record(b);

    const sgl::Rect<float> area{.pos = {100.f, 50.f}, .size = {300.f, 240.f}};
    sgl::RenderQueue queue;
    renderTrendChart(queue, history, area, defaultTheme(), {});

    std::size_t textCount = 0;
    for(const sgl::Entry& e: queue.sorted())
    {
        if(std::holds_alternative<sgl::TextCmd>(e.command))
        {
            ++textCount;
        }
        if(const auto* mesh = std::get_if<sgl::MeshCmd>(&e.command))
        {
            for(const sgl::Vertex& v: mesh->vertices)
            {
                EXPECT_TRUE(insideInclusive(v.position, area))
                    << "vertex (" << v.position.x << ',' << v.position.y << ") outside area";
            }
        }
    }
    EXPECT_GE(textCount, 6u);

    const auto spots = chartHotspots(history, area);
    ASSERT_EQ(spots.size(), 6u);
    EXPECT_EQ(spots[2].title, "Biotech");
    EXPECT_NE(spots[2].body.find("112"), std::string::npos);
    EXPECT_NE(spots[2].body.find("+12"), std::string::npos);
    EXPECT_NE(spots[2].body.find("x1.1"), std::string::npos);
}
