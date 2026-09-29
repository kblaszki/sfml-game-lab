#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <sgl/save/SaveFile.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <string>
#include <string_view>
#include <system_error>

namespace
{

[[nodiscard]] std::filesystem::path uniqueTempPath(std::string_view stem)
{
    const auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    return std::filesystem::temp_directory_path() / (std::string{stem} + stamp + ".txt");
}

} // namespace

TEST(ScoreStoreShould, missingFileYieldsEmptyTable)
{
    const auto path = uniqueTempPath("sgl_score_missing_");
    std::error_code ec;
    std::filesystem::remove(path, ec);

    const sgl::HighScoreTable table = sgl::ScoreStore::load(path, 5);
    EXPECT_TRUE(table.entries().empty());
}

TEST(ScoreStoreShould, corruptTextYieldsEmptyTable)
{
    const auto path = uniqueTempPath("sgl_score_corrupt_");
    ASSERT_TRUE(sgl::writeTextFileAtomic(path, "not-a-scores-file\n").has_value());

    const sgl::HighScoreTable table = sgl::ScoreStore::load(path, 3);
    EXPECT_TRUE(table.entries().empty());

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST(ScoreStoreShould, saveCreatesParentDirsAndRoundTrips)
{
    const auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto dir = std::filesystem::temp_directory_path() / ("sgl_score_dir_" + stamp) / "nested";
    const auto path = dir / "scores.txt";

    sgl::HighScoreTable table{5};
    ASSERT_TRUE(table.insert(900).has_value());
    ASSERT_TRUE(table.insert(400).has_value());

    const auto written = sgl::ScoreStore::save(path, table);
    ASSERT_TRUE(written.has_value()) << sgl::describe(written.error());

    const sgl::HighScoreTable loaded = sgl::ScoreStore::load(path, 5);
    ASSERT_EQ(loaded.entries().size(), 2u);
    EXPECT_EQ(loaded.entries()[0], 900u);
    EXPECT_EQ(loaded.entries()[1], 400u);

    std::error_code ec;
    std::filesystem::remove_all(dir.parent_path(), ec);
}
