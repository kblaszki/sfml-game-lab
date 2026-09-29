#include <gtest/gtest.h>
#include <sgl/resources/Atlas.hpp>
#include <string>

TEST(AtlasShould, ParseNamedRegionWithSlash)
{
    const auto manifest = sgl::parseAtlas("sector/ai 0 0 64 64\n");
    ASSERT_TRUE(manifest.has_value());
    EXPECT_EQ(manifest->size(), 1u);
    const auto rect = manifest->find("sector/ai");
    ASSERT_TRUE(rect.has_value());
    EXPECT_EQ(rect->x, 0);
    EXPECT_EQ(rect->y, 0);
    EXPECT_EQ(rect->w, 64);
    EXPECT_EQ(rect->h, 64);
}

TEST(AtlasShould, RejectNegativeWidth)
{
    const auto manifest = sgl::parseAtlas("bad 0 0 -1 64\n");
    ASSERT_FALSE(manifest.has_value());
    EXPECT_NE(manifest.error().find("width"), std::string::npos);
}

TEST(AtlasShould, RejectMissingField)
{
    const auto manifest = sgl::parseAtlas("incomplete 1 2 3\n");
    ASSERT_FALSE(manifest.has_value());
    EXPECT_NE(manifest.error().find("missing"), std::string::npos);
}

TEST(AtlasShould, RejectDuplicateName)
{
    const auto manifest = sgl::parseAtlas("sector/ai 0 0 64 64\nsector/ai 10 10 32 32\n");
    ASSERT_FALSE(manifest.has_value());
    EXPECT_NE(manifest.error().find("duplicate"), std::string::npos);
}
