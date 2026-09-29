#include <gtest/gtest.h>
#include <sgl/anim/Easing.hpp>
#include <sgl/anim/Spline.hpp>
#include <sgl/anim/Tween.hpp>
#include <sgl/core/Vec2.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

TEST(AnimEasingTest, EndpointsAreIdentity)
{
    EXPECT_FLOAT_EQ(sgl::linear(0.f), 0.f);
    EXPECT_FLOAT_EQ(sgl::linear(1.f), 1.f);
    EXPECT_FLOAT_EQ(sgl::easeInQuad(0.f), 0.f);
    EXPECT_FLOAT_EQ(sgl::easeInQuad(1.f), 1.f);
    EXPECT_FLOAT_EQ(sgl::easeOutCubic(0.f), 0.f);
    EXPECT_FLOAT_EQ(sgl::easeOutCubic(1.f), 1.f);
    EXPECT_FLOAT_EQ(sgl::easeInOutCubic(0.f), 0.f);
    EXPECT_FLOAT_EQ(sgl::easeInOutCubic(1.f), 1.f);
    EXPECT_FLOAT_EQ(sgl::easeOutBack(0.f), 0.f);
    EXPECT_FLOAT_EQ(sgl::easeOutBack(1.f), 1.f);
}

TEST(AnimEasingTest, EaseInOutCubicMonotonicAndMidpoint)
{
    const float a = sgl::easeInOutCubic(0.f);
    const float b = sgl::easeInOutCubic(0.25f);
    const float c = sgl::easeInOutCubic(0.5f);
    const float d = sgl::easeInOutCubic(0.75f);
    const float e = sgl::easeInOutCubic(1.f);
    EXPECT_LE(a, b);
    EXPECT_LE(b, c);
    EXPECT_LE(c, d);
    EXPECT_LE(d, e);
    EXPECT_FLOAT_EQ(c, 0.5f);
}

TEST(AnimEasingTest, ValuesOutsideUnitIntervalClamp)
{
    EXPECT_FLOAT_EQ(sgl::linear(-1.f), 0.f);
    EXPECT_FLOAT_EQ(sgl::linear(2.f), 1.f);
}

TEST(AnimTweenTest, LinearHalfwayThenCompletes)
{
    sgl::Tween<float> tween;
    EXPECT_TRUE(tween.done());
    EXPECT_FLOAT_EQ(tween.value(), 0.f);

    tween.start(0.f, 10.f, sgl::Seconds{1.f}, sgl::linear);
    EXPECT_FALSE(tween.done());
    EXPECT_TRUE(tween.running());

    tween.update(sgl::Seconds{0.5f});
    EXPECT_NEAR(tween.value(), 5.f, 1e-5f);
    EXPECT_FALSE(tween.done());

    tween.update(sgl::Seconds{1.f});
    EXPECT_FLOAT_EQ(tween.value(), 10.f);
    EXPECT_TRUE(tween.done());
    EXPECT_FALSE(tween.running());

    tween.update(sgl::Seconds{1.f});
    EXPECT_FLOAT_EQ(tween.value(), 10.f);
    EXPECT_TRUE(tween.done());
}

TEST(AnimTweenTest, ZeroDurationSnapsToTarget)
{
    sgl::Tween<float> tween;
    tween.start(0.f, 42.f, sgl::Seconds{0.f}, sgl::linear);
    EXPECT_FLOAT_EQ(tween.value(), 42.f);
    EXPECT_TRUE(tween.done());
    EXPECT_FALSE(tween.running());
}

TEST(AnimSplineTest, CatmullRomIncludesControlPoints)
{
    const std::vector<sgl::Vec2f> points{{0.f, 0.f}, {10.f, 5.f}, {20.f, 0.f}, {30.f, 8.f}};
    const auto samples = sgl::catmullRom(points, 4);
    ASSERT_FALSE(samples.empty());
    EXPECT_EQ(samples.front(), points.front());
    EXPECT_EQ(samples.back(), points.back());

    auto nearPoint = [&](sgl::Vec2f target) {
        for(const auto& s : samples)
        {
            if(std::abs(s.x - target.x) <= 1e-3f && std::abs(s.y - target.y) <= 1e-3f)
            {
                return true;
            }
        }
        return false;
    };

    EXPECT_TRUE(nearPoint(points[1]));
    EXPECT_TRUE(nearPoint(points[2]));
}

TEST(AnimSplineTest, ArcLengthTablePolylineLookups)
{
    const std::vector<sgl::Vec2f> poly{{0.f, 0.f}, {100.f, 0.f}, {100.f, 100.f}};
    sgl::ArcLengthTable table;
    table.build(poly);

    EXPECT_NEAR(table.length(), 200.f, 1e-2f);
    EXPECT_NEAR(table.at(0.f).x, 0.f, 1e-2f);
    EXPECT_NEAR(table.at(0.f).y, 0.f, 1e-2f);
    EXPECT_NEAR(table.at(table.length()).x, 100.f, 1e-2f);
    EXPECT_NEAR(table.at(table.length()).y, 100.f, 1e-2f);

    EXPECT_NEAR(table.at(50.f).x, 50.f, 1e-2f);
    EXPECT_NEAR(table.at(50.f).y, 0.f, 1e-2f);
    EXPECT_NEAR(table.at(100.f).x, 100.f, 1e-2f);
    EXPECT_NEAR(table.at(100.f).y, 0.f, 1e-2f);
    EXPECT_NEAR(table.at(150.f).x, 100.f, 1e-2f);
    EXPECT_NEAR(table.at(150.f).y, 50.f, 1e-2f);
}

TEST(AnimSplineTest, EqualDistanceStepsOnStraightLine)
{
    const std::vector<sgl::Vec2f> line{{0.f, 0.f}, {100.f, 0.f}};
    sgl::ArcLengthTable table;
    table.build(line);
    ASSERT_NEAR(table.length(), 100.f, 1e-3f);

    sgl::Vec2f prev = table.at(0.f);
    float minChord = 1e9f;
    float maxChord = 0.f;
    for(int i = 1; i <= 10; ++i)
    {
        const sgl::Vec2f cur = table.at(static_cast<float>(i) * 10.f);
        const sgl::Vec2f d = cur - prev;
        const float chord = std::sqrt(sgl::lengthSq(d));
        minChord = std::min(minChord, chord);
        maxChord = std::max(maxChord, chord);
        prev = cur;
    }
    EXPECT_LE(maxChord - minChord, 0.05f * maxChord);
}
