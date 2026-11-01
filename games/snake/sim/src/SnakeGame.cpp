#include <snake/sim/SnakeGame.hpp>

namespace sgl::snake
{
namespace
{

[[nodiscard]] bool opposite(Dir a, Dir b) noexcept
{
    switch(a)
    {
        case Dir::Up:
            return b == Dir::Down;
        case Dir::Down:
            return b == Dir::Up;
        case Dir::Left:
            return b == Dir::Right;
        case Dir::Right:
            return b == Dir::Left;
    }
    return false;
}

[[nodiscard]] Cell ahead(Cell cell, Dir dir) noexcept
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

[[nodiscard]] bool inBounds(Cell cell) noexcept
{
    return cell.x >= 0 && cell.y >= 0 && cell.x < kCols && cell.y < kRows;
}

} // namespace

SnakeGame::SnakeGame(std::uint64_t seed, Tuning tuning)
    : tuning_{tuning}
    , rng_{seed}
{
    const int length = tuning_.startLength;
    const int headX = kCols / 2;
    const int headY = kRows / 2;
    if(length > 0)
    {
        body_.reserve(static_cast<std::size_t>(length));
        for(int i = 0; i < length; ++i)
        {
            body_.push_back(Cell{headX - i, headY});
        }
    }
    placeFood();
}

std::vector<SnakeEvent> SnakeGame::step(const SimInput& input, sgl::Seconds dt)
{
    std::vector<SnakeEvent> events;
    if(over_ || body_.empty())
    {
        return events;
    }

    applyTurn(input.turn);
    moveAccum_ += dt;
    while(moveAccum_ >= tuning_.moveInterval && !over_)
    {
        moveAccum_ -= tuning_.moveInterval;
        advance(events);
    }
    return events;
}

const std::vector<Cell>& SnakeGame::body() const noexcept
{
    return body_;
}

Cell SnakeGame::food() const noexcept
{
    return food_;
}

Dir SnakeGame::dir() const noexcept
{
    return dir_;
}

int SnakeGame::score() const noexcept
{
    return score_;
}

bool SnakeGame::over() const noexcept
{
    return over_;
}

void SnakeGame::applyTurn(std::optional<Dir> turn)
{
    if(!turn.has_value() || opposite(dir_, *turn))
    {
        return;
    }
    dir_ = *turn;
}

void SnakeGame::advance(std::vector<SnakeEvent>& events)
{
    const Cell next = ahead(body_.front(), dir_);
    if(!inBounds(next))
    {
        over_ = true;
        events.push_back(Died{});
        return;
    }

    const bool eating = hasFood_ && next == food_;
    if(hitsSelf(next, eating))
    {
        over_ = true;
        events.push_back(Died{});
        return;
    }

    body_.insert(body_.begin(), next);
    if(!eating)
    {
        body_.pop_back();
        events.push_back(Moved{});
        return;
    }

    ++score_;
    placeFood();
    events.push_back(Ate{score_});
}

void SnakeGame::placeFood()
{
    int freeCount = 0;
    for(int y = 0; y < kRows; ++y)
    {
        for(int x = 0; x < kCols; ++x)
        {
            if(!occupied(Cell{x, y}))
            {
                ++freeCount;
            }
        }
    }
    if(freeCount == 0)
    {
        hasFood_ = false;
        food_ = Cell{-1, -1};
        return;
    }

    int pick = static_cast<int>(rng_.uniformBelow(static_cast<std::uint32_t>(freeCount)));
    for(int y = 0; y < kRows; ++y)
    {
        for(int x = 0; x < kCols; ++x)
        {
            const Cell cell{x, y};
            if(occupied(cell))
            {
                continue;
            }
            if(pick == 0)
            {
                food_ = cell;
                hasFood_ = true;
                return;
            }
            --pick;
        }
    }

    hasFood_ = false;
    food_ = Cell{-1, -1};
}

bool SnakeGame::occupied(Cell cell) const
{
    for(const Cell& part: body_)
    {
        if(part == cell)
        {
            return true;
        }
    }
    return false;
}

bool SnakeGame::hitsSelf(Cell next, bool eating) const
{
    if(body_.empty())
    {
        return false;
    }
    const std::size_t count = eating ? body_.size() : body_.size() - 1;
    for(std::size_t i = 0; i < count; ++i)
    {
        if(body_[i] == next)
        {
            return true;
        }
    }
    return false;
}

} // namespace sgl::snake
