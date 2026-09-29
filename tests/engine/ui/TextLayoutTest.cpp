#include <gtest/gtest.h>
#include <sgl/ui/TextLayout.hpp>
#include <string>

TEST(TextLayoutShould, estimateWidthFromByteCount)
{
    EXPECT_FLOAT_EQ(sgl::estimateWidth("abcd", 10), 4.f * 10.f * 0.6f);
    EXPECT_FLOAT_EQ(sgl::estimateWidth("ab", 20, 0.5f), 2.f * 20.f * 0.5f);
}

TEST(TextLayoutShould, wrapEmptyTextToEmptyVector)
{
    EXPECT_TRUE(sgl::wrap("", 10, 100.f).empty());
}

TEST(TextLayoutShould, wrapOnSpacesWithoutSplittingWords)
{
    constexpr unsigned kSize = 10;
    // "hello" and "world" are each 30px; "hello world" is 66px.
    constexpr float kMaxWidth = 30.f;

    const auto lines = sgl::wrap("hello world", kSize, kMaxWidth);
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "hello");
    EXPECT_EQ(lines[1], "world");
    EXPECT_NEAR(sgl::estimateWidth(lines[0], kSize), kMaxWidth, 0.01f);
    EXPECT_NEAR(sgl::estimateWidth(lines[1], kSize), kMaxWidth, 0.01f);
}

TEST(TextLayoutShould, keepOversizedWordWhole)
{
    constexpr unsigned kSize = 10;
    constexpr float kMaxWidth = 20.f;
    const std::string word = "supercalifragilistic";

    const auto lines = sgl::wrap(word, kSize, kMaxWidth);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], word);
    EXPECT_GT(sgl::estimateWidth(lines[0], kSize), kMaxWidth);
}
