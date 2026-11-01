#pragma once

#include <cstdint>

namespace sgl::snake
{

struct Cell
{
    int x{};
    int y{};

    friend bool operator==(Cell, Cell) = default;
};

enum class Dir : std::uint8_t
{
    Up,
    Down,
    Left,
    Right
};

inline constexpr int kCols = 32;
inline constexpr int kRows = 18;

} // namespace sgl::snake
