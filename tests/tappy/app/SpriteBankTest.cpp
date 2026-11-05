#include <gtest/gtest.h>
#include <sgl/resources/Atlas.hpp>
#include <tappy/app/SpriteBank.hpp>
#include <vector>

TEST(TappySpriteBankShould, EmptyReturnsNullopt)
{
    const sgl::tappy::SpriteBank bank{};
    const auto cmd =
        bank.sprite("player/blue", sgl::Vec2f{0.f, 0.f}, sgl::Vec2f{48.f, 36.f}, sgl::Color{255, 255, 255});
    EXPECT_EQ(cmd, std::nullopt);
}

TEST(TappySpriteBankShould, ReturnsSpriteCmdWithRotation)
{
    sgl::AtlasManifest manifest{std::vector<sgl::AtlasEntry>{
        sgl::AtlasEntry{.name = "player/blue", .rect = sgl::AtlasRect{.x = 8, .y = 16, .w = 64, .h = 32}},
    }};
    const sgl::TextureId texture{1};
    const sgl::tappy::SpriteBank bank{texture, std::move(manifest)};

    const auto cmd =
        bank.sprite("player/blue", sgl::Vec2f{100.f, 200.f}, sgl::Vec2f{32.f, 16.f}, sgl::Color{10, 20, 30, 40}, 15.f);
    ASSERT_TRUE(cmd.has_value());
    EXPECT_EQ(cmd->texture, texture);
    EXPECT_FLOAT_EQ(cmd->rotationDeg, 15.f);
    EXPECT_FLOAT_EQ(cmd->scale.x, 0.5f);
}
