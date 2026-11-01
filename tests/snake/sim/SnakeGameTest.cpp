#include <cstdlib>
#include <gtest/gtest.h>
#include <optional>
#include <snake/sim/SnakeGame.hpp>
#include <variant>
#include <vector>

namespace
{

using sgl::Seconds;
using sgl::snake::Ate;
using sgl::snake::Cell;
using sgl::snake::Dir;
using sgl::snake::kCols;
using sgl::snake::kRows;
using sgl::snake::Moved;
using sgl::snake::SimInput;
using sgl::snake::SnakeEvent;
using sgl::snake::SnakeGame;
using sgl::snake::Tuning;

[[nodiscard]] bool opposite(Dir a, Dir b)
{
    return (a == Dir::Up && b == Dir::Down) || (a == Dir::Down && b == Dir::Up) || (a == Dir::Left && b == Dir::Right)
           || (a == Dir::Right && b == Dir::Left);
}

[[nodiscard]] Cell ahead(Cell cell, Dir dir)
{
    switch(dir)
    {
        case Dir::Up:
            return Cell{cell.x, cell.y + 1};
        case Dir::Down:
            return Cell{cell.x, cell.y - 1};
        case Dir::Left:
            return Cell{cell.x - 1, cell.y};
        case Dir::Right:
            return Cell{cell.x + 1, cell.y};
    }
    return cell;
}

[[nodiscard]] bool inBounds(Cell cell)
{
    return cell.x >= 0 && cell.y >= 0 && cell.x < kCols && cell.y < kRows;
}

[[nodiscard]] int manhattan(Cell a, Cell b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

[[nodiscard]] bool blocks(const SnakeGame& game, Cell cell)
{
    const std::vector<Cell>& body = game.body();
    if(body.empty())
    {
        return false;
    }
    const std::size_t tail = body.size() - 1;
    for(std::size_t i = 0; i < body.size(); ++i)
    {
        if(body[i] == cell && !(i == tail && cell != game.food()))
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] Dir chooseTurn(const SnakeGame& game)
{
    const Cell head = game.body().front();
    const Cell food = game.food();
    const Dir current = game.dir();
    const Dir options[]{Dir::Up, Dir::Down, Dir::Left, Dir::Right};

    std::optional<Dir> best;
    int bestDist = 0;
    for(const Dir dir: options)
    {
        if(opposite(dir, current))
        {
            continue;
        }
        const Cell next = ahead(head, dir);
        if(!inBounds(next) || blocks(game, next))
        {
            continue;
        }
        const int dist = manhattan(next, food);
        if(!best.has_value() || dist < bestDist || (dist == bestDist && dir == current))
        {
            best = dir;
            bestDist = dist;
        }
    }
    return best.value_or(current);
}

} // namespace

TEST(SnakeGameShould, startsNearCenterFacingRight)
{
    const SnakeGame game{1};
    ASSERT_EQ(game.body().size(), 3u);
    EXPECT_EQ(game.dir(), Dir::Right);
    EXPECT_EQ(game.score(), 0);
    EXPECT_FALSE(game.over());
    EXPECT_EQ(game.body()[0], (Cell{kCols / 2, kRows / 2}));
    EXPECT_EQ(game.body()[1], (Cell{kCols / 2 - 1, kRows / 2}));
    EXPECT_EQ(game.body()[2], (Cell{kCols / 2 - 2, kRows / 2}));
    EXPECT_GE(game.food().x, 0);
    EXPECT_LT(game.food().x, kCols);
    EXPECT_GE(game.food().y, 0);
    EXPECT_LT(game.food().y, kRows);
    for(const Cell& part: game.body())
    {
        EXPECT_NE(part, game.food());
    }
}

TEST(SnakeGameShould, movesHeadOneCellAfterInterval)
{
    const Tuning tuning{};
    SnakeGame game{1, tuning};
    const Cell head = game.body().front();

    const std::vector<SnakeEvent> early = game.step({}, tuning.moveInterval / 2);
    EXPECT_TRUE(early.empty());
    EXPECT_EQ(game.body().front(), head);

    const std::vector<SnakeEvent> events = game.step({}, tuning.moveInterval / 2);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<Moved>(events[0]) || std::holds_alternative<Ate>(events[0]));
    EXPECT_EQ(game.body().front(), (Cell{head.x + 1, head.y}));
    EXPECT_FALSE(game.over());

    const Cell mid = game.body().front();
    game.step({}, tuning.moveInterval * 2);
    EXPECT_EQ(game.body().front(), (Cell{mid.x + 2, mid.y}));
    EXPECT_FALSE(game.over());
}

TEST(SnakeGameShould, ignoresReverseTurn)
{
    const Tuning tuning{};
    SnakeGame game{1, tuning};
    const Cell head = game.body().front();
    SimInput input{};
    input.turn = Dir::Left;

    const std::vector<SnakeEvent> events = game.step(input, tuning.moveInterval);
    EXPECT_EQ(game.dir(), Dir::Right);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(game.body().front(), (Cell{head.x + 1, head.y}));
}

TEST(SnakeGameShould, appliesPerpendicularTurnBeforeMoving)
{
    const Tuning tuning{};
    SnakeGame game{1, tuning};
    const Cell head = game.body().front();
    SimInput input{};
    input.turn = Dir::Up;

    const std::vector<SnakeEvent> held = game.step(input, Seconds{0.f});
    EXPECT_TRUE(held.empty());
    EXPECT_EQ(game.dir(), Dir::Up);
    EXPECT_EQ(game.body().front(), head);

    game.step({}, tuning.moveInterval);
    EXPECT_EQ(game.dir(), Dir::Up);
    EXPECT_EQ(game.body().front(), (Cell{head.x, head.y + 1}));
}

TEST(SnakeGameShould, growsAndScoresWhenEating)
{
    const Tuning tuning{};
    SnakeGame game{3, tuning};
    const std::size_t lengthBefore = game.body().size();
    const Cell foodBefore = game.food();

    std::vector<SnakeEvent> last;
    bool ate = false;
    for(int i = 0; i < kCols * kRows; ++i)
    {
        SimInput input{};
        input.turn = chooseTurn(game);
        last = game.step(input, tuning.moveInterval);
        if(game.score() == 1)
        {
            ate = true;
            break;
        }
        ASSERT_FALSE(game.over()) << "died on step " << i;
    }

    ASSERT_TRUE(ate);
    EXPECT_EQ(game.body().size(), lengthBefore + 1);
    EXPECT_EQ(game.body().front(), foodBefore);
    EXPECT_NE(game.food(), foodBefore);
    EXPECT_GE(game.food().x, 0);
    EXPECT_LT(game.food().x, kCols);
    EXPECT_GE(game.food().y, 0);
    EXPECT_LT(game.food().y, kRows);
    for(const Cell& part: game.body())
    {
        EXPECT_NE(part, game.food());
    }
    ASSERT_FALSE(last.empty());
    const Ate* ateEvent = std::get_if<Ate>(&last.back());
    ASSERT_NE(ateEvent, nullptr);
    EXPECT_EQ(ateEvent->score, 1);
    EXPECT_FALSE(game.over());
}

TEST(SnakeGameShould, diesOnWall)
{
    const Tuning tuning{};
    SnakeGame game{11, tuning};
    const int steps = kCols - game.body().front().x;
    std::vector<SnakeEvent> events;
    for(int i = 0; i < steps; ++i)
    {
        events = game.step({}, tuning.moveInterval);
        if(game.over())
        {
            break;
        }
    }

    EXPECT_TRUE(game.over());
    ASSERT_EQ(events.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<sgl::snake::Died>(events[0]));
    EXPECT_EQ(game.body().front(), (Cell{kCols - 1, kRows / 2}));

    const std::vector<Cell> body = game.body();
    const std::vector<SnakeEvent> after = game.step({}, tuning.moveInterval);
    EXPECT_TRUE(after.empty());
    EXPECT_EQ(game.body(), body);
    EXPECT_TRUE(game.over());
}

TEST(SnakeGameShould, diesOnSelf)
{
    Tuning tuning{};
    tuning.startLength = 5;
    SnakeGame game{1, tuning};

    const auto turn = [&](Dir dir) {
        SimInput input{};
        input.turn = dir;
        return game.step(input, tuning.moveInterval);
    };

    turn(Dir::Up);
    ASSERT_FALSE(game.over());
    EXPECT_EQ(game.dir(), Dir::Up);
    turn(Dir::Left);
    ASSERT_FALSE(game.over());
    EXPECT_EQ(game.dir(), Dir::Left);

    const std::vector<Cell> before = game.body();
    const std::vector<SnakeEvent> down = turn(Dir::Down);
    EXPECT_TRUE(game.over());
    ASSERT_EQ(down.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<sgl::snake::Died>(down[0]));
    EXPECT_EQ(game.body(), before);

    const std::vector<SnakeEvent> after = game.step({}, tuning.moveInterval);
    EXPECT_TRUE(after.empty());
    EXPECT_EQ(game.body(), before);
}

TEST(SnakeGameShould, matchesBodyFoodAndScoreForSameSeed)
{
    constexpr std::uint64_t seed = 42;
    constexpr int steps = 80;
    const Tuning tuning{};
    SnakeGame first{seed, tuning};
    SnakeGame second{seed, tuning};

    for(int i = 0; i < steps && !first.over(); ++i)
    {
        SimInput input{};
        input.turn = chooseTurn(first);
        SimInput other{};
        other.turn = chooseTurn(second);
        first.step(input, tuning.moveInterval);
        second.step(other, tuning.moveInterval);
    }

    EXPECT_EQ(first.body(), second.body());
    EXPECT_EQ(first.food(), second.food());
    EXPECT_EQ(first.score(), second.score());
    EXPECT_EQ(first.dir(), second.dir());
    EXPECT_EQ(first.over(), second.over());
    EXPECT_GT(first.score(), 0);
}
