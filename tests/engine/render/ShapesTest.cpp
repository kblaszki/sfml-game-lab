#include <cmath>
#include <gtest/gtest.h>
#include <sgl/render/Shapes.hpp>

namespace
{

bool insideRect(const sgl::Vec2f& p, const sgl::Rect<float>& rect, float tol = 1e-3f)
{
    return p.x >= rect.pos.x - tol && p.x <= rect.pos.x + rect.size.x + tol && p.y >= rect.pos.y - tol &&
           p.y <= rect.pos.y + rect.size.y + tol;
}

} // namespace

TEST(ShapesTest, RoundedRectRadiusZeroHasSixVertsInside)
{
    const sgl::Rect<float> rect{.pos = {10.f, 20.f}, .size = {40.f, 30.f}};
    const sgl::MeshCmd mesh = sgl::roundedRect(rect, 0.f, {255, 255, 255, 255});
    ASSERT_EQ(mesh.vertices.size(), 6u);
    EXPECT_EQ(mesh.kind, sgl::MeshKind::Triangles);
    for(const sgl::Vertex& v: mesh.vertices)
    {
        EXPECT_TRUE(insideRect(v.position, rect));
    }
}

TEST(ShapesTest, RoundedRectVertexCountAndBounds)
{
    const sgl::Rect<float> rect{.pos = {0.f, 0.f}, .size = {100.f, 60.f}};
    constexpr int segments = 6;
    const sgl::MeshCmd mesh = sgl::roundedRect(rect, 10.f, {200, 100, 50, 255}, segments);
    ASSERT_EQ(mesh.vertices.size(), static_cast<std::size_t>(18 + 12 * segments));
    for(const sgl::Vertex& v: mesh.vertices)
    {
        EXPECT_TRUE(insideRect(v.position, rect));
    }
}

TEST(ShapesTest, VerticalGradientTopAndBottomColors)
{
    const sgl::Rect<float> rect{.pos = {0.f, 0.f}, .size = {10.f, 20.f}};
    const sgl::Color top{255, 0, 0, 255};
    const sgl::Color bottom{0, 0, 255, 255};
    const sgl::MeshCmd mesh = sgl::verticalGradient(rect, top, bottom);
    ASSERT_EQ(mesh.vertices.size(), 6u);

    for(const sgl::Vertex& v: mesh.vertices)
    {
        if(std::abs(v.position.y - rect.pos.y) < 1e-3f)
        {
            EXPECT_EQ(v.color, top);
        }
        if(std::abs(v.position.y - (rect.pos.y + rect.size.y)) < 1e-3f)
        {
            EXPECT_EQ(v.color, bottom);
        }
    }
}

TEST(ShapesTest, RadialGlowAlphasAndCount)
{
    constexpr int segments = 16;
    const sgl::Color inner{10, 20, 30, 200};
    const sgl::MeshCmd mesh = sgl::radialGlow({0.f, 0.f}, 5.f, 15.f, inner, segments);
    ASSERT_EQ(mesh.vertices.size(), static_cast<std::size_t>(3 * segments));
    for(std::size_t i = 0; i < mesh.vertices.size(); i += 3)
    {
        EXPECT_EQ(mesh.vertices[i].color.a, inner.a);
        EXPECT_EQ(mesh.vertices[i].color.r, inner.r);
        EXPECT_EQ(mesh.vertices[i + 1].color.a, 0);
        EXPECT_EQ(mesh.vertices[i + 1].color.r, inner.r);
        EXPECT_EQ(mesh.vertices[i + 2].color.a, 0);
        EXPECT_EQ(mesh.vertices[i + 2].color.r, inner.r);
    }
}

TEST(ShapesTest, RadialGlowOuterNotGreaterReturnsEmpty)
{
    const sgl::MeshCmd mesh = sgl::radialGlow({0.f, 0.f}, 10.f, 10.f, {255, 255, 255, 255}, 32);
    EXPECT_TRUE(mesh.vertices.empty());
}

TEST(ShapesTest, PolylineHorizontalColinear)
{
    const sgl::Vec2f points[] = {{0.f, 5.f}, {10.f, 5.f}, {25.f, 5.f}};
    constexpr float thickness = 4.f;
    const sgl::MeshCmd mesh = sgl::polyline(points, thickness, {255, 255, 255, 255});
    ASSERT_GE(mesh.vertices.size(), 12u);
    EXPECT_EQ(mesh.vertices.size() % 6u, 0u);
    const float half = thickness * 0.5f;
    for(const sgl::Vertex& v: mesh.vertices)
    {
        EXPECT_FALSE(std::isnan(v.position.x));
        EXPECT_FALSE(std::isnan(v.position.y));
        EXPECT_NEAR(v.position.y, 5.f, half + 1e-3f);
    }
}

TEST(ShapesTest, ArrowHeadTipAndDirection)
{
    const sgl::Vec2f tip{50.f, 10.f};
    const sgl::MeshCmd mesh = sgl::arrowHead(tip, {1.f, 0.f}, 8.f, {255, 0, 0, 255});
    ASSERT_EQ(mesh.vertices.size(), 3u);

    bool foundTip = false;
    for(const sgl::Vertex& v: mesh.vertices)
    {
        if(v.position == tip)
        {
            foundTip = true;
        }
        else
        {
            EXPECT_LT(v.position.x, tip.x);
        }
    }
    EXPECT_TRUE(foundTip);
}
