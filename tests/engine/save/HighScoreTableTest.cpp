#include <gtest/gtest.h>
#include <sgl/save/HighScoreTable.hpp>

TEST(HighScoreTableTest, keepsDescendingOrder)
{
    sgl::HighScoreTable table(5);
    EXPECT_EQ(table.insert(50), 0u);
    EXPECT_EQ(table.insert(100), 0u);
    EXPECT_EQ(table.insert(75), 1u);

    const auto entries = table.entries();
    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries[0], 100u);
    EXPECT_EQ(entries[1], 75u);
    EXPECT_EQ(entries[2], 50u);
}

TEST(HighScoreTableTest, dropsBeyondCapacity)
{
    sgl::HighScoreTable table(2);
    EXPECT_TRUE(table.insert(100).has_value());
    EXPECT_TRUE(table.insert(90).has_value());
    EXPECT_FALSE(table.insert(80).has_value());

    const auto entries = table.entries();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0], 100u);
    EXPECT_EQ(entries[1], 90u);

    EXPECT_TRUE(table.insert(95).has_value());
    const auto after = table.entries();
    ASSERT_EQ(after.size(), 2u);
    EXPECT_EQ(after[0], 100u);
    EXPECT_EQ(after[1], 95u);
}

TEST(HighScoreTableTest, tieKeepsOlderFirst)
{
    sgl::HighScoreTable table(3);
    EXPECT_EQ(table.insert(100), 0u);
    EXPECT_EQ(table.insert(100), 1u);

    const auto entries = table.entries();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0], 100u);
    EXPECT_EQ(entries[1], 100u);
}

TEST(HighScoreTableTest, qualifiesWhenNotFull)
{
    sgl::HighScoreTable table(3);
    EXPECT_TRUE(table.qualifies(0));
    EXPECT_TRUE(table.insert(10).has_value());
    EXPECT_TRUE(table.qualifies(1));
    EXPECT_TRUE(table.insert(20).has_value());
    EXPECT_TRUE(table.insert(30).has_value());
    EXPECT_FALSE(table.qualifies(10));
    EXPECT_TRUE(table.qualifies(31));
}

TEST(HighScoreTableTest, capacityZeroRejectsInsertAndStaysEmpty)
{
    sgl::HighScoreTable table(0);
    EXPECT_FALSE(table.qualifies(100));
    EXPECT_FALSE(table.insert(100).has_value());
    EXPECT_TRUE(table.entries().empty());
}
