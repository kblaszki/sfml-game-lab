#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sgl::syndicate
{

enum class NodeId : std::uint16_t
{
};

[[nodiscard]] constexpr std::strong_ordering operator<=>(NodeId a, NodeId b) noexcept
{
    return std::to_underlying(a) <=> std::to_underlying(b);
}

enum class PlayerId : std::uint8_t
{
};

inline constexpr std::size_t kMaxPlayers = 4;

enum class Sector : std::uint8_t
{
    AI,
    CleanEnergy,
    Biotech,
    CyberSec,
    Logistics,
    Fintech,
};

} // namespace sgl::syndicate
