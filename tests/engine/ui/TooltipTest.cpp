#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/ui/TooltipSystem.hpp>
#include <vector>

namespace
{

[[nodiscard]] sgl::Hotspot makeHotspot(sgl::Rect<float> area, const char* title, const char* body)
{
    return sgl::Hotspot{.area = area, .title = title, .body = body};
}

} // namespace

TEST(TooltipSystemShould, showAfterDelayAndHideOnLeave)
{
    sgl::TooltipSystem system;
    const sgl::TooltipStyle style{};
    const std::vector<sgl::Hotspot> hotspots{
        makeHotspot({.pos = {100.f, 100.f}, .size = {80.f, 40.f}}, "Title", "body text here"),
    };
    const sgl::Vec2f pointer{120.f, 110.f};

    system.update(pointer, hotspots, sgl::Seconds{0.34f}, style);
    EXPECT_FALSE(system.active().has_value());

    system.update(pointer, hotspots, sgl::Seconds{0.02f}, style);
    ASSERT_TRUE(system.active().has_value());
    EXPECT_EQ(system.active()->title, "Title");

    system.update(sgl::Vec2f{0.f, 0.f}, hotspots, sgl::Seconds{0.016f}, style);
    EXPECT_FALSE(system.active().has_value());
}

TEST(TooltipSystemShould, keepPanelInsideScreenNearCorner)
{
    sgl::TooltipSystem system;
    sgl::TooltipStyle style{};
    style.maxWidth = 300.f;
    style.delay = 0.f;

    const std::vector<sgl::Hotspot> hotspots{
        makeHotspot({.pos = {1200.f, 680.f}, .size = {80.f, 40.f}},
                    "Tip",
                    "wide tooltip body that forces a large panel width for clamping"),
    };
    const sgl::Vec2f pointer{1270.f, 710.f};

    system.update(pointer, hotspots, sgl::Seconds{0.02f}, style);
    ASSERT_TRUE(system.active().has_value());

    const sgl::Rect<float>& panel = system.active()->panel;
    EXPECT_LE(panel.pos.x + panel.size.x, 1280.f);
    EXPECT_LE(panel.pos.y + panel.size.y, 720.f);
    EXPECT_GE(panel.pos.x, 0.f);
    EXPECT_GE(panel.pos.y, 0.f);
}

TEST(TooltipSystemShould, resetDelayWhenSwitchingHotspots)
{
    sgl::TooltipSystem system;
    const sgl::TooltipStyle style{};
    const std::vector<sgl::Hotspot> hotspots{
        makeHotspot({.pos = {0.f, 0.f}, .size = {50.f, 50.f}}, "A", "first"),
        makeHotspot({.pos = {100.f, 0.f}, .size = {50.f, 50.f}}, "B", "second"),
    };

    system.update(sgl::Vec2f{10.f, 10.f}, hotspots, sgl::Seconds{0.40f}, style);
    ASSERT_TRUE(system.active().has_value());
    EXPECT_EQ(system.active()->title, "A");

    system.update(sgl::Vec2f{110.f, 10.f}, hotspots, sgl::Seconds{0.016f}, style);
    EXPECT_FALSE(system.active().has_value());

    system.update(sgl::Vec2f{110.f, 10.f}, hotspots, sgl::Seconds{0.30f}, style);
    EXPECT_FALSE(system.active().has_value());

    system.update(sgl::Vec2f{110.f, 10.f}, hotspots, sgl::Seconds{0.05f}, style);
    ASSERT_TRUE(system.active().has_value());
    EXPECT_EQ(system.active()->title, "B");
}
