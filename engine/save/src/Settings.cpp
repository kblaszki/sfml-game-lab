#include <algorithm>
#include <charconv>
#include <sgl/save/SaveFile.hpp>
#include <sgl/save/Settings.hpp>
#include <sstream>
#include <system_error>

namespace sgl
{
namespace
{

constexpr std::string_view kHeader = "sgl-settings 1";

[[nodiscard]] std::string_view trimCr(std::string_view line) noexcept
{
    if(!line.empty() && line.back() == '\r')
    {
        line.remove_suffix(1);
    }
    return line;
}

[[nodiscard]] bool isKnownResolution(Vec2u resolution) noexcept
{
    for(const Vec2u& known: kResolutions)
    {
        if(known == resolution)
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool parseInt(std::string_view text, int& out) noexcept
{
    if(text.empty())
    {
        return false;
    }
    const auto* begin = text.data();
    const auto* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, out);
    return result.ec == std::errc{} && result.ptr == end;
}

[[nodiscard]] bool parseResolution(std::string_view text, Vec2u& out) noexcept
{
    const auto xPos = text.find('x');
    if(xPos == std::string_view::npos)
    {
        return false;
    }
    int width = 0;
    int height = 0;
    if(!parseInt(text.substr(0, xPos), width) || !parseInt(text.substr(xPos + 1), height))
    {
        return false;
    }
    if(width < 0 || height < 0)
    {
        return false;
    }
    out = Vec2u{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    return true;
}

} // namespace

std::expected<Settings, SaveError> parseSettings(std::string_view text)
{
    std::string_view remaining = text;
    const auto firstNl = remaining.find('\n');
    std::string_view header = firstNl == std::string_view::npos ? remaining : remaining.substr(0, firstNl);
    header = trimCr(header);
    if(header != kHeader)
    {
        return std::unexpected(SaveError::Corrupt);
    }

    Settings settings{};
    if(firstNl == std::string_view::npos)
    {
        return settings;
    }
    remaining.remove_prefix(firstNl + 1);

    while(!remaining.empty())
    {
        const auto nl = remaining.find('\n');
        std::string_view line = nl == std::string_view::npos ? remaining : remaining.substr(0, nl);
        line = trimCr(line);

        if(nl == std::string_view::npos)
        {
            remaining = {};
        }
        else
        {
            remaining.remove_prefix(nl + 1);
        }

        if(line.empty())
        {
            continue;
        }

        const auto eq = line.find('=');
        if(eq == std::string_view::npos)
        {
            continue;
        }
        const std::string_view key = line.substr(0, eq);
        const std::string_view value = line.substr(eq + 1);

        if(key == "volume")
        {
            int volume = 0;
            if(parseInt(value, volume))
            {
                settings.masterVolume = std::clamp(volume, 0, 100);
            }
        }
        else if(key == "mode")
        {
            if(value == "windowed")
            {
                settings.mode = WindowMode::Windowed;
            }
            else if(value == "fullscreen")
            {
                settings.mode = WindowMode::Fullscreen;
            }
        }
        else if(key == "resolution")
        {
            Vec2u resolution{};
            if(parseResolution(value, resolution) && isKnownResolution(resolution))
            {
                settings.resolution = resolution;
            }
        }
    }

    return settings;
}

std::string serializeSettings(const Settings& settings)
{
    std::ostringstream out;
    out << kHeader << '\n';
    out << "volume=" << std::clamp(settings.masterVolume, 0, 100) << '\n';
    out << "mode=" << (settings.mode == WindowMode::Fullscreen ? "fullscreen" : "windowed") << '\n';
    out << "resolution=" << settings.resolution.x << 'x' << settings.resolution.y << '\n';
    return out.str();
}

Settings SettingsStore::load(const std::filesystem::path& path)
{
    const auto text = readTextFile(path);
    if(!text)
    {
        return Settings{};
    }

    const auto settings = parseSettings(*text);
    if(!settings)
    {
        return Settings{};
    }
    return *settings;
}

std::expected<void, SaveError> SettingsStore::save(const std::filesystem::path& path, const Settings& settings)
{
    const auto parent = path.parent_path();
    if(!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if(ec)
        {
            return std::unexpected(SaveError::Io);
        }
    }
    return writeTextFileAtomic(path, serializeSettings(settings));
}

} // namespace sgl
