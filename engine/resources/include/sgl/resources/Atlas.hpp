#pragma once

#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace sgl
{

struct AtlasRect
{
    int x{};
    int y{};
    int w{};
    int h{};

    constexpr bool operator==(const AtlasRect&) const = default;
};

struct AtlasEntry
{
    std::string name;
    AtlasRect rect;
};

class AtlasManifest
{
public:
    AtlasManifest() = default;
    explicit AtlasManifest(std::vector<AtlasEntry> entries);

    [[nodiscard]] std::optional<AtlasRect> find(std::string_view name) const;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<AtlasEntry> entries_{};
};

[[nodiscard]] std::expected<AtlasManifest, std::string> parseAtlas(std::string_view text);

} // namespace sgl
