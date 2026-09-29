#include <algorithm>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <tetris/sim/Events.hpp>
#include <tetris/sim/Grid.hpp>
#include <tetris/sim/Piece.hpp>
#include <tetris/sim/TetrisGame.hpp>
#include <tetris/sim/Types.hpp>
#include <vector>

namespace
{

using sgl::Seconds;
using sgl::tetris::Cells;
using sgl::tetris::Coord;
using sgl::tetris::GameOver;
using sgl::tetris::Grid;
using sgl::tetris::HardDropped;
using sgl::tetris::LinesCleared;
using sgl::tetris::PieceLocked;
using sgl::tetris::PieceType;
using sgl::tetris::Rotation;
using sgl::tetris::TetrisGame;
using sgl::tetris::TetrisInput;
using sgl::tetris::Tuning;

TetrisInput none()
{
    return {};
}

bool hasEvent(const std::vector<sgl::tetris::TetrisEvent>& events, auto pred)
{
    return std::ranges::any_of(events, pred);
}

Grid& mutableGrid(TetrisGame& game)
{
    return const_cast<Grid&>(game.grid());
}

void lockPadded(Grid& grid, std::vector<Coord> cells)
{
    int pad = 0;
    auto nextPad = [&]() {
        while(true)
        {
            const Coord c{pad % Grid::kWidth, 36 + pad / Grid::kWidth};
            ++pad;
            if(!grid.occupied(c) && std::find(cells.begin(), cells.end(), c) == cells.end())
            {
                return c;
            }
        }
    };
    while(cells.size() % 4 != 0)
    {
        cells.push_back(nextPad());
    }
    for(std::size_t i = 0; i < cells.size(); i += 4)
    {
        const Cells four{cells[i], cells[i + 1], cells[i + 2], cells[i + 3]};
        ASSERT_TRUE(grid.fits(four));
        grid.lock(four, PieceType::I);
    }
}

void fillCell(Grid& grid, Coord c)
{
    lockPadded(grid, {c});
}

void fillRow(Grid& grid, int y)
{
    lockPadded(grid, {{0, y}, {1, y}, {2, y}, {3, y}, {4, y}, {5, y}, {6, y}, {7, y}, {8, y}, {9, y}});
}

void fillRowExcept(Grid& grid, int y, int emptyX)
{
    std::vector<Coord> cells;
    for(int x = 0; x < Grid::kWidth; ++x)
    {
        if(x != emptyX)
        {
            cells.push_back({x, y});
        }
    }
    lockPadded(grid, std::move(cells));
}

void groundPiece(TetrisGame& game)
{
    TetrisInput soft{};
    soft.softDropHeld = true;
    for(int i = 0; i < 2000 && game.active().has_value(); ++i)
    {
        if(!sgl::tetris::tryShift(game.grid(), *game.active(), Coord{0, -1}).has_value())
        {
            break;
        }
        game.step(soft, Seconds{0.05f});
    }
}

int pieceOriginX(const TetrisGame& game)
{
    return game.active()->origin.x;
}

} // namespace

TEST(TetrisGameTest, tapShiftsOnce)
{
    TetrisGame game{1};
    const int x0 = pieceOriginX(game);
    TetrisInput in{};
    in.leftPressed = true;
    game.step(in, Seconds{0.f});
    EXPECT_EQ(pieceOriginX(game), x0 - 1);
    game.step(none(), Seconds{0.016f});
    EXPECT_EQ(pieceOriginX(game), x0 - 1);
}

TEST(TetrisGameTest, dasStartsAfterDelay)
{
    Tuning tuning{};
    tuning.das = Seconds{0.2f};
    tuning.arr = Seconds{0.05f};
    TetrisGame game{1, tuning};
    const int x0 = pieceOriginX(game);

    TetrisInput hold{};
    hold.leftPressed = true;
    hold.leftHeld = true;
    game.step(hold, Seconds{0.f});
    EXPECT_EQ(pieceOriginX(game), x0 - 1);

    hold.leftPressed = false;
    game.step(hold, Seconds{0.19f});
    EXPECT_EQ(pieceOriginX(game), x0 - 1);

    game.step(hold, Seconds{0.02f});
    EXPECT_EQ(pieceOriginX(game), x0 - 2);
}

TEST(TetrisGameTest, arrRepeatsAtRate)
{
    Tuning tuning{};
    tuning.das = Seconds{0.1f};
    tuning.arr = Seconds{0.05f};
    TetrisGame game{1, tuning};
    const int x0 = pieceOriginX(game);

    TetrisInput hold{};
    hold.rightPressed = true;
    hold.rightHeld = true;
    game.step(hold, Seconds{0.f});
    EXPECT_EQ(pieceOriginX(game), x0 + 1);
    hold.rightPressed = false;
    game.step(hold, Seconds{0.1f});
    EXPECT_EQ(pieceOriginX(game), x0 + 2);

    game.step(hold, Seconds{0.05f});
    EXPECT_EQ(pieceOriginX(game), x0 + 3);
    game.step(hold, Seconds{0.05f});
    EXPECT_EQ(pieceOriginX(game), x0 + 4);
}

TEST(TetrisGameTest, arrZeroTeleportsToWall)
{
    Tuning tuning{};
    tuning.das = Seconds{0.1f};
    tuning.arr = Seconds{0.f};
    TetrisGame game{1, tuning};

    TetrisInput hold{};
    hold.leftPressed = true;
    hold.leftHeld = true;
    game.step(hold, Seconds{0.f});
    hold.leftPressed = false;
    game.step(hold, Seconds{0.1f});

    const auto c = sgl::tetris::cells(*game.active());
    const int minX = std::ranges::min_element(c, {}, &Coord::x)->x;
    EXPECT_EQ(minX, 0);
}

TEST(TetrisGameTest, lastPressedDirectionWins)
{
    TetrisGame game{1};
    const int x0 = pieceOriginX(game);

    TetrisInput in{};
    in.leftPressed = true;
    in.leftHeld = true;
    game.step(in, Seconds{0.f});
    EXPECT_EQ(pieceOriginX(game), x0 - 1);

    in.leftPressed = false;
    in.rightPressed = true;
    in.rightHeld = true;
    game.step(in, Seconds{0.f});
    EXPECT_EQ(pieceOriginX(game), x0);
}

TEST(TetrisGameTest, hardDropLocksImmediately)
{
    TetrisGame game{1};
    TetrisInput in{};
    in.hardDrop = true;
    const auto events = game.step(in, Seconds{0.f});
    EXPECT_TRUE(hasEvent(events, [](const auto& e) { return std::holds_alternative<HardDropped>(e); }));
    EXPECT_TRUE(hasEvent(events, [](const auto& e) { return std::holds_alternative<PieceLocked>(e); }));
    EXPECT_TRUE(game.active().has_value());
}

TEST(TetrisGameTest, lockDelayResetsOnMove)
{
    Tuning tuning{};
    tuning.lockDelay = Seconds{0.5f};
    tuning.das = Seconds{10.f};
    TetrisGame game{1, tuning};
    groundPiece(game);
    ASSERT_TRUE(game.active().has_value());

    game.step(none(), Seconds{0.4f});
    EXPECT_TRUE(game.active().has_value());

    TetrisInput move{};
    move.rightPressed = true;
    game.step(move, Seconds{0.f});
    EXPECT_TRUE(game.active().has_value());

    game.step(none(), Seconds{0.4f});
    EXPECT_TRUE(game.active().has_value()) << "lock delay should have been reset by the move";

    const auto events = game.step(none(), Seconds{0.2f});
    EXPECT_TRUE(hasEvent(events, [](const auto& e) { return std::holds_alternative<PieceLocked>(e); }));
}

TEST(TetrisGameTest, lockResetLimitFifteen)
{
    Tuning tuning{};
    tuning.lockDelay = Seconds{0.5f};
    tuning.das = Seconds{10.f};
    TetrisGame game{1, tuning};
    groundPiece(game);
    ASSERT_TRUE(game.active().has_value());

    for(int i = 0; i < 15; ++i)
    {
        TetrisInput move{};
        move.leftPressed = (i % 2 == 0);
        move.rightPressed = (i % 2 == 1);
        game.step(move, Seconds{0.f});
        ASSERT_TRUE(game.active().has_value()) << i;
    }

    game.step(none(), Seconds{0.49f});
    EXPECT_TRUE(game.active().has_value());

    TetrisInput move{};
    move.leftPressed = true;
    game.step(move, Seconds{0.f});
    const auto events = game.step(none(), Seconds{0.02f});
    EXPECT_TRUE(hasEvent(events, [](const auto& e) { return std::holds_alternative<PieceLocked>(e); }));
}

TEST(TetrisGameTest, newLowestRowRestoresResets)
{
    Tuning tuning{};
    tuning.lockDelay = Seconds{0.5f};
    tuning.das = Seconds{10.f};
    TetrisGame game{1, tuning};

    TetrisInput soft{};
    soft.softDropHeld = true;
    for(int i = 0; i < 2000 && game.active().has_value(); ++i)
    {
        const auto one = sgl::tetris::tryShift(game.grid(), *game.active(), Coord{0, -1});
        if(!one.has_value())
        {
            break;
        }
        const auto two = sgl::tetris::tryShift(game.grid(), *one, Coord{0, -1});
        if(!two.has_value())
        {
            break;
        }
        game.step(soft, Seconds{0.05f});
    }

    for(int i = 0; i < 10; ++i)
    {
        TetrisInput move{};
        move.leftPressed = (i % 2 == 0);
        move.rightPressed = (i % 2 == 1);
        game.step(move, Seconds{0.f});
        ASSERT_TRUE(game.active().has_value());
    }

    ASSERT_TRUE(sgl::tetris::tryShift(game.grid(), *game.active(), Coord{0, -1}).has_value());
    game.step(soft, Seconds{0.05f});
    ASSERT_TRUE(game.active().has_value());

    for(int i = 0; i < 15; ++i)
    {
        TetrisInput move{};
        move.leftPressed = (i % 2 == 0);
        move.rightPressed = (i % 2 == 1);
        game.step(move, Seconds{0.f});
        ASSERT_TRUE(game.active().has_value()) << "restored resets allow move " << i;
    }
}

TEST(TetrisGameTest, holdOncePerPiece)
{
    TetrisGame game{1};
    const PieceType first = game.active()->type;

    TetrisInput hold{};
    hold.hold = true;
    game.step(hold, Seconds{0.f});
    EXPECT_EQ(game.held(), first);
    const PieceType second = game.active()->type;

    game.step(hold, Seconds{0.f});
    EXPECT_EQ(game.active()->type, second);
    EXPECT_EQ(game.held(), first);
}

TEST(TetrisGameTest, holdReturnsSpawnOrientation)
{
    TetrisGame game{1};
    TetrisInput rot{};
    rot.rotateCw = true;
    game.step(rot, Seconds{0.f});
    ASSERT_NE(game.active()->rotation, Rotation::Spawn);
    const PieceType heldType = game.active()->type;

    TetrisInput hold{};
    hold.hold = true;
    game.step(hold, Seconds{0.f});

    TetrisInput hard{};
    hard.hardDrop = true;
    game.step(hard, Seconds{0.f});

    game.step(hold, Seconds{0.f});
    ASSERT_TRUE(game.active().has_value());
    EXPECT_EQ(game.active()->type, heldType);
    EXPECT_EQ(game.active()->rotation, Rotation::Spawn);
}

TEST(TetrisGameTest, blockOutEndsGame)
{
    TetrisGame game{1};
    const PieceType nextType = game.nextQueue()[0];
    Grid& grid = mutableGrid(game);
    for(Coord c: sgl::tetris::cells(sgl::tetris::spawnPiece(nextType)))
    {
        if(!grid.occupied(c))
        {
            fillCell(grid, c);
        }
    }

    TetrisInput hard{};
    hard.hardDrop = true;
    const auto events = game.step(hard, Seconds{0.f});
    EXPECT_TRUE(game.over());
    EXPECT_TRUE(hasEvent(events, [](const auto& e) { return std::holds_alternative<GameOver>(e); }));
}

TEST(TetrisGameTest, lockOutEndsGame)
{
    Tuning tuning{};
    tuning.lockDelay = Seconds{0.1f};
    TetrisGame game{1, tuning};
    fillRow(mutableGrid(game), 19);
    groundPiece(game);
    ASSERT_TRUE(game.active().has_value());
    const auto c = sgl::tetris::cells(*game.active());
    ASSERT_TRUE(std::ranges::all_of(c, [](Coord cell) { return cell.y > 19; }));

    const auto events = game.step(none(), Seconds{0.2f});
    EXPECT_TRUE(game.over());
    EXPECT_TRUE(hasEvent(events, [](const auto& e) { return std::holds_alternative<PieceLocked>(e); }));
    EXPECT_TRUE(hasEvent(events, [](const auto& e) { return std::holds_alternative<GameOver>(e); }));
}

TEST(TetrisGameTest, tetrisScores800AtLevel1)
{
    TetrisGame game{1};
    Grid& grid = mutableGrid(game);
    for(int y = 0; y < 4; ++y)
    {
        fillRowExcept(grid, y, 0);
    }

    for(int n = 0; n < 14 && game.active()->type != PieceType::I; ++n)
    {
        if(!game.held().has_value() || game.held() != PieceType::I)
        {
            TetrisInput hold{};
            hold.hold = true;
            game.step(hold, Seconds{0.f});
        }
        if(game.active()->type == PieceType::I)
        {
            break;
        }
        TetrisInput hard{};
        hard.hardDrop = true;
        game.step(hard, Seconds{0.f});
        for(int y = 0; y < 4; ++y)
        {
            for(int x = 1; x < Grid::kWidth; ++x)
            {
                if(!grid.occupied({x, y}))
                {
                    fillCell(grid, {x, y});
                }
            }
        }
    }
    ASSERT_EQ(game.active()->type, PieceType::I);

    TetrisInput rot{};
    rot.rotateCw = true;
    game.step(rot, Seconds{0.f});

    for(int i = 0; i < 10; ++i)
    {
        const auto c = sgl::tetris::cells(*game.active());
        const int minX = std::ranges::min_element(c, {}, &Coord::x)->x;
        if(minX <= 0)
        {
            break;
        }
        TetrisInput left{};
        left.leftPressed = true;
        game.step(left, Seconds{0.f});
    }

    const std::uint32_t pointsBefore = game.score().points;
    const auto beforeCells = sgl::tetris::cells(*game.active());
    const auto afterCells = sgl::tetris::cells(sgl::tetris::dropToFloor(game.grid(), *game.active()));
    const int beforeMinY = std::ranges::min_element(beforeCells, {}, &Coord::y)->y;
    const int afterMinY = std::ranges::min_element(afterCells, {}, &Coord::y)->y;
    const int rows = beforeMinY - afterMinY;

    TetrisInput hard{};
    hard.hardDrop = true;
    const auto events = game.step(hard, Seconds{0.f});
    EXPECT_TRUE(hasEvent(events, [](const auto& e) {
        if(!std::holds_alternative<LinesCleared>(e))
        {
            return false;
        }
        const auto& cleared = std::get<LinesCleared>(e);
        return cleared.count == 4 && cleared.rows[0] == 0 && cleared.rows[1] == 1 && cleared.rows[2] == 2
               && cleared.rows[3] == 3;
    }));
    EXPECT_EQ(game.score().points, pointsBefore + static_cast<std::uint32_t>(rows) * 2u + 800u);
    EXPECT_EQ(game.score().level, 1u);
}

TEST(TetrisGameTest, pressedAndReleasedWithinOneTickStillShifts)
{
    TetrisGame game{1};
    const int x0 = pieceOriginX(game);
    TetrisInput in{};
    in.leftPressed = true;
    in.leftHeld = false;
    game.step(in, Seconds{0.f});
    EXPECT_EQ(pieceOriginX(game), x0 - 1);
}
