#pragma once

#include <array>
#include <cstdint>
#include <variant>

namespace sgl::tetris
{

struct PieceSpawned
{
};

struct PieceShifted
{
};

struct PieceRotated
{
};

struct PieceLocked
{
};

struct LinesCleared
{
    std::array<std::uint8_t, 4> rows{};
    std::uint8_t count{};
};

struct HardDropped
{
};

struct HoldUsed
{
};

struct LevelUp
{
};

struct GameOver
{
};

using TetrisEvent = std::variant<PieceSpawned,
                                 PieceShifted,
                                 PieceRotated,
                                 PieceLocked,
                                 LinesCleared,
                                 HardDropped,
                                 HoldUsed,
                                 LevelUp,
                                 GameOver>;

} // namespace sgl::tetris
