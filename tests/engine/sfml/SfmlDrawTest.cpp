#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <cmath>
#include <gtest/gtest.h>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/sfml/SfmlDraw.hpp>

TEST(SfmlDrawTest, rotationAroundCenterKeepsTopLeftAtZero)
{
    const sgl::RectCmd cmd{
        .rect = {.pos = {0.f, 0.f}, .size = {40.f, 20.f}},
        .fill = {255, 255, 255, 255},
        .rotationDeg = 0.f,
    };
    const sf::RectangleShape unrotated = sgl::sfml::toShape(cmd);
    EXPECT_FLOAT_EQ(unrotated.getOrigin().x, 20.f);
    EXPECT_FLOAT_EQ(unrotated.getOrigin().y, 10.f);
    EXPECT_FLOAT_EQ(unrotated.getPosition().x, 20.f);
    EXPECT_FLOAT_EQ(unrotated.getPosition().y, 10.f);

    const sf::Vector2f topLeft = unrotated.getTransform().transformPoint({0.f, 0.f});
    EXPECT_FLOAT_EQ(topLeft.x, 0.f);
    EXPECT_FLOAT_EQ(topLeft.y, 0.f);

    sgl::RectCmd rotated = cmd;
    rotated.rotationDeg = 90.f;
    const sf::RectangleShape shape = sgl::sfml::toShape(rotated);
    EXPECT_FLOAT_EQ(shape.getOrigin().x, 20.f);
    EXPECT_FLOAT_EQ(shape.getOrigin().y, 10.f);
    EXPECT_FLOAT_EQ(shape.getPosition().x, 20.f);
    EXPECT_FLOAT_EQ(shape.getPosition().y, 10.f);
    EXPECT_FLOAT_EQ(shape.getRotation().asDegrees(), 90.f);

    // Center of the unrotated bounds stays fixed under rotation.
    const sf::Vector2f center = shape.getTransform().transformPoint({20.f, 10.f});
    EXPECT_NEAR(center.x, 20.f, 1e-4f);
    EXPECT_NEAR(center.y, 10.f, 1e-4f);
}

TEST(SfmlDrawTest, lineAtZeroDegrees)
{
    const sgl::LineCmd cmd{
        .from = {10.f, 20.f},
        .to = {50.f, 20.f},
        .thickness = 4.f,
        .color = {255, 0, 0, 255},
    };
    const sf::RectangleShape shape = sgl::sfml::toShape(cmd);
    EXPECT_FLOAT_EQ(shape.getSize().x, 40.f);
    EXPECT_FLOAT_EQ(shape.getSize().y, 4.f);
    EXPECT_FLOAT_EQ(shape.getOrigin().x, 0.f);
    EXPECT_FLOAT_EQ(shape.getOrigin().y, 2.f);
    EXPECT_FLOAT_EQ(shape.getPosition().x, 10.f);
    EXPECT_FLOAT_EQ(shape.getPosition().y, 20.f);
    EXPECT_NEAR(shape.getRotation().asDegrees(), 0.f, 1e-4f);
}

TEST(SfmlDrawTest, lineAtNinetyDegrees)
{
    const sgl::LineCmd cmd{
        .from = {0.f, 0.f},
        .to = {0.f, 30.f},
        .thickness = 2.f,
        .color = {0, 255, 0, 255},
    };
    const sf::RectangleShape shape = sgl::sfml::toShape(cmd);
    EXPECT_FLOAT_EQ(shape.getSize().x, 30.f);
    EXPECT_FLOAT_EQ(shape.getSize().y, 2.f);
    EXPECT_FLOAT_EQ(shape.getOrigin().x, 0.f);
    EXPECT_FLOAT_EQ(shape.getOrigin().y, 1.f);
    EXPECT_FLOAT_EQ(shape.getPosition().x, 0.f);
    EXPECT_FLOAT_EQ(shape.getPosition().y, 0.f);
    EXPECT_NEAR(shape.getRotation().asDegrees(), 90.f, 1e-4f);
}

TEST(SfmlDrawTest, lineAtFortyFiveDegrees)
{
    const sgl::LineCmd cmd{
        .from = {0.f, 0.f},
        .to = {10.f, 10.f},
        .thickness = 1.f,
        .color = {0, 0, 255, 255},
    };
    const sf::RectangleShape shape = sgl::sfml::toShape(cmd);
    const float expectedLen = std::sqrt(200.f);
    EXPECT_NEAR(shape.getSize().x, expectedLen, 1e-4f);
    EXPECT_FLOAT_EQ(shape.getSize().y, 1.f);
    EXPECT_FLOAT_EQ(shape.getPosition().x, 0.f);
    EXPECT_FLOAT_EQ(shape.getPosition().y, 0.f);
    EXPECT_NEAR(shape.getRotation().asDegrees(), 45.f, 1e-4f);
}

TEST(SfmlDrawTest, circleCenterAndRadius)
{
    const sgl::CircleCmd cmd{
        .center = {100.f, 50.f},
        .radius = 12.f,
        .fill = {255, 255, 255, 255},
        .outline = {0, 0, 0, 255},
        .outlineThickness = 2.f,
        .points = 16,
    };
    const sf::CircleShape shape = sgl::sfml::toShape(cmd);
    EXPECT_FLOAT_EQ(shape.getRadius(), 12.f);
    EXPECT_FLOAT_EQ(shape.getOrigin().x, 12.f);
    EXPECT_FLOAT_EQ(shape.getOrigin().y, 12.f);
    EXPECT_FLOAT_EQ(shape.getPosition().x, 100.f);
    EXPECT_FLOAT_EQ(shape.getPosition().y, 50.f);
    EXPECT_EQ(shape.getPointCount(), 16u);
    EXPECT_FLOAT_EQ(shape.getOutlineThickness(), 2.f);
}
