#include <gtest/gtest.h>
#include <sgl/resources/Atlas.hpp>
#include <syndicate/app/SpriteBank.hpp>
#include <vector>

TEST(SpriteBankShould, EmptyReturnsNullopt)
{
    const sgl::syndicate::SpriteBank bank{};
    const auto cmd = bank.sprite("anything", sgl::Vec2f{0.f, 0.f}, sgl::Vec2f{16.f, 16.f}, sgl::Color{255, 255, 255});
    EXPECT_EQ(cmd, std::nullopt);
}

TEST(SpriteBankShould, ReturnsSpriteCmdMatchingEntry)
{
    sgl::AtlasManifest manifest{std::vector<sgl::AtlasEntry>{
        sgl::AtlasEntry{.name = "sector/ai", .rect = sgl::AtlasRect{.x = 8, .y = 16, .w = 64, .h = 64}},
    }};
    const sgl::TextureId texture{1};
    const sgl::syndicate::SpriteBank bank{texture, std::move(manifest)};

    const auto cmd =
        bank.sprite("sector/ai", sgl::Vec2f{100.f, 200.f}, sgl::Vec2f{32.f, 48.f}, sgl::Color{10, 20, 30, 40});
    ASSERT_TRUE(cmd.has_value());
    EXPECT_EQ(cmd->texture, texture);
    EXPECT_FLOAT_EQ(cmd->source.pos.x, 8.f);
    EXPECT_FLOAT_EQ(cmd->source.pos.y, 16.f);
    EXPECT_FLOAT_EQ(cmd->source.size.x, 64.f);
    EXPECT_FLOAT_EQ(cmd->source.size.y, 64.f);
    EXPECT_FLOAT_EQ(cmd->position.x, 100.f);
    EXPECT_FLOAT_EQ(cmd->position.y, 200.f);
    EXPECT_FLOAT_EQ(cmd->scale.x, 0.5f);
    EXPECT_FLOAT_EQ(cmd->scale.y, 0.75f);
    EXPECT_EQ(cmd->tint, (sgl::Color{10, 20, 30, 40}));
    EXPECT_FLOAT_EQ(cmd->rotationDeg, 0.f);
}
