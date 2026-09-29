#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <sgl/save/SaveFile.hpp>
#include <string>
#include <string_view>
#include <system_error>

TEST(SaveFileTest, writeThenReadInTempDir)
{
    const auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("sgl_save_roundtrip_" + stamp + ".txt");
    const auto tmpPath = std::filesystem::path(path.native() + std::filesystem::path(".tmp").native());

    const std::string_view payload = "hello\nworld\n";
    const auto writeResult = sgl::writeTextFileAtomic(path, payload);
    ASSERT_TRUE(writeResult.has_value()) << sgl::describe(writeResult.error());

    const auto readResult = sgl::readTextFile(path);
    ASSERT_TRUE(readResult.has_value()) << sgl::describe(readResult.error());
    EXPECT_EQ(*readResult, payload);

    std::error_code ec;
    std::filesystem::remove(path, ec);
    std::filesystem::remove(tmpPath, ec);
}

TEST(SaveFileTest, overwritesExistingFile)
{
    const auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("sgl_save_overwrite_" + stamp + ".txt");
    const auto tmpPath = std::filesystem::path(path.native() + std::filesystem::path(".tmp").native());

    ASSERT_TRUE(sgl::writeTextFileAtomic(path, "A").has_value());
    ASSERT_TRUE(sgl::writeTextFileAtomic(path, "B").has_value());

    const auto readResult = sgl::readTextFile(path);
    ASSERT_TRUE(readResult.has_value()) << sgl::describe(readResult.error());
    EXPECT_EQ(*readResult, "B");

    std::error_code ec;
    std::filesystem::remove(path, ec);
    std::filesystem::remove(tmpPath, ec);
}

TEST(SaveFileTest, missingFileIsNotFound)
{
    const auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("sgl_save_missing_" + stamp + ".txt");
    std::error_code ec;
    std::filesystem::remove(path, ec);

    const auto result = sgl::readTextFile(path);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), sgl::SaveError::NotFound);
}
