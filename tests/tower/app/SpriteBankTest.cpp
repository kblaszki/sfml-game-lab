#include <gtest/gtest.h>
#include <sgl/resources/Atlas.hpp>
#include <tower/app/SpriteBank.hpp>
#include <vector>

TEST(TowerSpriteBankShould, EmptyReturnsNullopt)
{
    const sgl::tower::SpriteBank bank{};
    const auto cmd =
        bank.sprite("player/idle", sgl::Vec2f{0.f, 0.f}, sgl::Vec2f{40.f, 56.f}, sgl::Color{255, 255, 255});
    EXPECT_EQ(cmd, std::nullopt);
}

TEST(TowerSpriteBankShould, ReturnsSpriteCmdWithRotation)
{
    sgl::AtlasManifest manifest{std::vector<sgl::AtlasEntry>{
        sgl::AtlasEntry{.name = "player/idle", .rect = sgl::AtlasRect{.x = 8, .y = 16, .w = 64, .h = 32}},
    }};
    const sgl::TextureId texture{1};
    const sgl::tower::SpriteBank bank{texture, std::move(manifest)};

    const auto cmd =
        bank.sprite("player/idle", sgl::Vec2f{100.f, 200.f}, sgl::Vec2f{32.f, 16.f}, sgl::Color{10, 20, 30, 40}, 45.f);
    ASSERT_TRUE(cmd.has_value());
    EXPECT_EQ(cmd->texture, texture);
    EXPECT_FLOAT_EQ(cmd->source.pos.x, 8.f);
    EXPECT_FLOAT_EQ(cmd->source.pos.y, 16.f);
    EXPECT_FLOAT_EQ(cmd->source.size.x, 64.f);
    EXPECT_FLOAT_EQ(cmd->source.size.y, 32.f);
    EXPECT_FLOAT_EQ(cmd->position.x, 100.f);
    EXPECT_FLOAT_EQ(cmd->position.y, 200.f);
    EXPECT_FLOAT_EQ(cmd->scale.x, 0.5f);
    EXPECT_FLOAT_EQ(cmd->scale.y, 0.5f);
    EXPECT_EQ(cmd->tint, (sgl::Color{10, 20, 30, 40}));
    EXPECT_FLOAT_EQ(cmd->rotationDeg, 45.f);

    const auto upright =
        bank.sprite("player/idle", sgl::Vec2f{100.f, 200.f}, sgl::Vec2f{32.f, 16.f}, sgl::Color{255, 255, 255});
    ASSERT_TRUE(upright.has_value());
    EXPECT_FLOAT_EQ(upright->rotationDeg, 0.f);
}
