#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <sgl/save/SaveError.hpp>
#include <sgl/save/Settings.hpp>
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

TEST(SettingsShould, serializeParseRoundTrip)
{
    sgl::Settings settings{};
    settings.masterVolume = 55;
    settings.mode = sgl::WindowMode::Fullscreen;
    settings.resolution = {1920, 1080};

    const std::string text = sgl::serializeSettings(settings);
    const auto parsed = sgl::parseSettings(text);
    ASSERT_TRUE(parsed.has_value()) << sgl::describe(parsed.error());
    EXPECT_EQ(*parsed, settings);
}

TEST(SettingsShould, badHeaderIsCorrupt)
{
    const auto result = sgl::parseSettings("not-settings\nvolume=80\n");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), sgl::SaveError::Corrupt);
}

TEST(SettingsShould, volume150ClampsTo100)
{
    const auto result = sgl::parseSettings("sgl-settings 1\nvolume=150\nmode=windowed\nresolution=1280x720\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->masterVolume, 100);
}

TEST(SettingsShould, unknownKeyIgnored)
{
    const auto result = sgl::parseSettings(
        "sgl-settings 1\nvolume=40\nfoo=1\nmode=fullscreen\nresolution=1600x900\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->masterVolume, 40);
    EXPECT_EQ(result->mode, sgl::WindowMode::Fullscreen);
    EXPECT_EQ(result->resolution, (sgl::Vec2u{1600, 900}));
}

TEST(SettingsShould, loadMissingPathReturnsDefaults)
{
    const auto path = uniqueTempPath("sgl_settings_missing_");
    std::error_code ec;
    std::filesystem::remove(path, ec);

    const sgl::Settings loaded = sgl::SettingsStore::load(path);
    EXPECT_EQ(loaded.masterVolume, 80);
    EXPECT_EQ(loaded.mode, sgl::WindowMode::Windowed);
    EXPECT_EQ(loaded.resolution, (sgl::Vec2u{1280, 720}));
}

TEST(SettingsShould, saveThenLoadRoundTrips)
{
    const auto path = uniqueTempPath("sgl_settings_roundtrip_");

    sgl::Settings settings{};
    settings.masterVolume = 25;
    settings.mode = sgl::WindowMode::Fullscreen;
    settings.resolution = {1600, 900};

    const auto written = sgl::SettingsStore::save(path, settings);
    ASSERT_TRUE(written.has_value()) << sgl::describe(written.error());

    const sgl::Settings loaded = sgl::SettingsStore::load(path);
    EXPECT_EQ(loaded, settings);

    std::error_code ec;
    std::filesystem::remove(path, ec);
}
