#include <cctype>
#include <sgl/resources/Atlas.hpp>
#include <sstream>
#include <utility>

namespace sgl
{
namespace
{

[[nodiscard]] std::string_view trim(std::string_view s)
{
    while(!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
    {
        s.remove_prefix(1);
    }
    while(!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
    {
        s.remove_suffix(1);
    }
    return s;
}

} // namespace

AtlasManifest::AtlasManifest(std::vector<AtlasEntry> entries) : entries_{std::move(entries)}
{
}

std::optional<AtlasRect> AtlasManifest::find(std::string_view name) const
{
    for(const AtlasEntry& entry: entries_)
    {
        if(entry.name == name)
        {
            return entry.rect;
        }
    }
    return std::nullopt;
}

std::size_t AtlasManifest::size() const noexcept
{
    return entries_.size();
}

std::expected<AtlasManifest, std::string> parseAtlas(std::string_view text)
{
    std::vector<AtlasEntry> entries;
    std::size_t lineStart = 0;
    while(lineStart <= text.size())
    {
        const std::size_t lineEnd = text.find('\n', lineStart);
        const std::string_view raw =
            lineEnd == std::string_view::npos ? text.substr(lineStart) : text.substr(lineStart, lineEnd - lineStart);
        lineStart = lineEnd == std::string_view::npos ? text.size() + 1 : lineEnd + 1;

        std::string_view line = trim(raw);
        if(!line.empty() && line.back() == '\r')
        {
            line.remove_suffix(1);
            line = trim(line);
        }
        if(line.empty() || line.front() == '#')
        {
            continue;
        }

        std::istringstream in{std::string{line}};
        std::string name;
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        if(!(in >> name >> x >> y >> w >> h))
        {
            return std::unexpected(std::string{"missing fields in atlas line"});
        }

        std::string extra;
        if(in >> extra)
        {
            return std::unexpected(std::string{"extra fields in atlas line"});
        }

        if(w <= 0 || h <= 0)
        {
            return std::unexpected(std::string{"negative or zero width/height"});
        }

        for(const AtlasEntry& existing: entries)
        {
            if(existing.name == name)
            {
                return std::unexpected(std::string{"duplicate name: "} + name);
            }
        }

        entries.push_back(AtlasEntry{.name = std::move(name), .rect = AtlasRect{.x = x, .y = y, .w = w, .h = h}});
    }

    return AtlasManifest{std::move(entries)};
}

} // namespace sgl
