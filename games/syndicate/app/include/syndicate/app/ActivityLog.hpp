#pragma once

#include <cstddef>
#include <optional>
#include <sgl/core/Rect.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <span>
#include <string>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/Ids.hpp>
#include <vector>

namespace sgl::syndicate
{

struct LogEntry
{
    int round{};
    std::optional<PlayerId> player;
    std::string text;
};

class ActivityLog
{
public:
    static constexpr std::size_t kCapacity = 64;

    void push(int round, std::optional<PlayerId> player, std::string text);

    [[nodiscard]] std::span<const LogEntry> visible(std::size_t n) const;

    void render(sgl::RenderQueue& queue, sgl::Rect<float> area, const Theme& theme, sgl::FontId font) const;

private:
    std::vector<LogEntry> entries_;
};

} // namespace sgl::syndicate
