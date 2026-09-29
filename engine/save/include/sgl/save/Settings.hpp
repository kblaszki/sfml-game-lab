#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <sgl/core/Vec2.hpp>
#include <sgl/save/SaveError.hpp>
#include <string>
#include <string_view>

namespace sgl
{

enum class WindowMode : std::uint8_t
{
    Windowed,
    Fullscreen
};

inline constexpr std::array<Vec2u, 3> kResolutions{{
    {1280, 720},
    {1600, 900},
    {1920, 1080},
}};

struct Settings
{
    int masterVolume{80}; // 0..100
    WindowMode mode{WindowMode::Windowed};
    Vec2u resolution{1280, 720};

    auto operator<=>(const Settings&) const = default;
};

[[nodiscard]] std::expected<Settings, SaveError> parseSettings(std::string_view text);

[[nodiscard]] std::string serializeSettings(const Settings& settings);

struct SettingsStore
{
    // Missing or corrupt -> defaults. Never throws.
    [[nodiscard]] static Settings load(const std::filesystem::path& path);

    [[nodiscard]] static std::expected<void, SaveError> save(const std::filesystem::path& path,
                                                             const Settings& settings);
};

} // namespace sgl
