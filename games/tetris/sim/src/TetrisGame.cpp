#include <algorithm>
#include <ranges>
#include <tetris/sim/TetrisGame.hpp>

namespace sgl::tetris
{
namespace
{

bool isLockOut(const Cells& locked)
{
    return std::ranges::all_of(locked, [](Coord c) { return c.y > 19; });
}

} // namespace

TetrisGame::TetrisGame(std::uint64_t seed, Tuning tuning)
    : tuning_{tuning}
    , bag_{seed}
{
    std::vector<TetrisEvent> discarded;
    spawnNext(discarded);
}

std::vector<TetrisEvent> TetrisGame::step(const TetrisInput& input, sgl::Seconds dt)
{
    std::vector<TetrisEvent> events;
    if(over_ || !active_.has_value())
    {
        return events;
    }

    handleHold(input, events);
    if(over_ || !active_.has_value())
    {
        return events;
    }

    handleRotations(input, events);
    handleHorizontal(input, dt, events);
    handleDrops(input, dt, events);
    if(over_ || !active_.has_value())
    {
        return events;
    }

    handleLock(dt, events);
    return events;
}

const Grid& TetrisGame::grid() const noexcept
{
    return grid_;
}

const std::optional<Piece>& TetrisGame::active() const noexcept
{
    return active_;
}

std::optional<Piece> TetrisGame::ghost() const
{
    if(!active_.has_value())
    {
        return std::nullopt;
    }
    return dropToFloor(grid_, *active_);
}

std::optional<PieceType> TetrisGame::held() const noexcept
{
    return held_;
}

std::array<PieceType, 5> TetrisGame::nextQueue() const
{
    return bag_.preview();
}

const Score& TetrisGame::score() const noexcept
{
    return score_;
}

bool TetrisGame::over() const noexcept
{
    return over_;
}

bool TetrisGame::grounded() const
{
    if(!active_.has_value())
    {
        return false;
    }
    return !tryShift(grid_, *active_, Coord{0, -1}).has_value();
}

int TetrisGame::lowestRow(const Piece& piece) const
{
    const Cells c = cells(piece);
    return std::ranges::min_element(c, {}, &Coord::y)->y;
}

void TetrisGame::noteLowest(const Piece& piece)
{
    const int row = lowestRow(piece);
    if(lowestRow_ < 0 || row < lowestRow_)
    {
        lowestRow_ = row;
        lockResets_ = 0;
        lockTimer_ = sgl::Seconds{0.f};
    }
}

bool TetrisGame::tryApplyShift(Coord delta, std::vector<TetrisEvent>& events)
{
    if(!active_.has_value())
    {
        return false;
    }
    const std::optional<Piece> moved = tryShift(grid_, *active_, delta);
    if(!moved.has_value())
    {
        return false;
    }

    const bool wasGrounded = grounded();
    active_ = *moved;
    noteLowest(*active_);
    events.push_back(PieceShifted{});

    if(wasGrounded && grounded() && delta.y == 0)
    {
        if(lockResets_ < tuning_.maxLockResets)
        {
            ++lockResets_;
            lockTimer_ = sgl::Seconds{0.f};
        }
    }
    return true;
}

bool TetrisGame::tryApplyRotate(Turn turn, std::vector<TetrisEvent>& events)
{
    if(!active_.has_value())
    {
        return false;
    }
    const std::optional<Piece> rotated = tryRotate(grid_, *active_, turn);
    if(!rotated.has_value())
    {
        return false;
    }

    const bool wasGrounded = grounded();
    active_ = *rotated;
    noteLowest(*active_);
    events.push_back(PieceRotated{});

    if(wasGrounded && grounded())
    {
        if(lockResets_ < tuning_.maxLockResets)
        {
            ++lockResets_;
            lockTimer_ = sgl::Seconds{0.f};
        }
    }
    return true;
}

void TetrisGame::handleHold(const TetrisInput& input, std::vector<TetrisEvent>& events)
{
    if(!input.hold || !holdAvailable_ || !active_.has_value())
    {
        return;
    }

    const PieceType current = active_->type;
    holdAvailable_ = false;
    events.push_back(HoldUsed{});

    if(!held_.has_value())
    {
        held_ = current;
        active_.reset();
        spawnNext(events);
    }
    else
    {
        const PieceType swap = *held_;
        held_ = current;
        active_.reset();
        spawnType(swap, events);
    }
}

void TetrisGame::handleRotations(const TetrisInput& input, std::vector<TetrisEvent>& events)
{
    if(input.rotateCcw)
    {
        tryApplyRotate(Turn::Ccw, events);
    }
    if(input.rotateCw)
    {
        tryApplyRotate(Turn::Cw, events);
    }
}

void TetrisGame::resetDas()
{
    dasTimer_ = sgl::Seconds{0.f};
    arrTimer_ = sgl::Seconds{0.f};
    dasCharged_ = false;
}

TetrisGame::Horizontal TetrisGame::resolveDirection(const TetrisInput& input)
{
    const bool leftEdge = input.leftPressed || (input.leftHeld && !prevLeftHeld_);
    const bool rightEdge = input.rightPressed || (input.rightHeld && !prevRightHeld_);

    if(leftEdge)
    {
        lastPressed_ = Horizontal::Left;
    }
    if(rightEdge)
    {
        lastPressed_ = Horizontal::Right;
    }

    const bool left = input.leftHeld || input.leftPressed;
    const bool right = input.rightHeld || input.rightPressed;

    if(left && right)
    {
        return lastPressed_ == Horizontal::None ? Horizontal::Left : lastPressed_;
    }
    if(left)
    {
        return Horizontal::Left;
    }
    if(right)
    {
        return Horizontal::Right;
    }
    return Horizontal::None;
}

void TetrisGame::handleHorizontal(const TetrisInput& input, sgl::Seconds dt, std::vector<TetrisEvent>& events)
{
    const bool leftEdge = input.leftPressed || (input.leftHeld && !prevLeftHeld_);
    const bool rightEdge = input.rightPressed || (input.rightHeld && !prevRightHeld_);

    const Horizontal dir = resolveDirection(input);

    if(dir == Horizontal::None)
    {
        dasDir_ = Horizontal::None;
        resetDas();
        prevLeftHeld_ = input.leftHeld;
        prevRightHeld_ = input.rightHeld;
        return;
    }

    const Coord delta{static_cast<int>(dir), 0};
    const bool edge = (dir == Horizontal::Left && leftEdge) || (dir == Horizontal::Right && rightEdge);

    if(dir != dasDir_ || edge)
    {
        dasDir_ = dir;
        resetDas();
        if(edge)
        {
            tryApplyShift(delta, events);
        }
    }

    if(input.leftHeld || input.rightHeld)
    {
        if(!dasCharged_)
        {
            dasTimer_ += dt;
            if(dasTimer_ >= tuning_.das)
            {
                dasCharged_ = true;
                arrTimer_ = sgl::Seconds{0.f};
                if(tuning_.arr == sgl::Seconds{0.f})
                {
                    while(tryApplyShift(delta, events))
                    {
                    }
                }
                else
                {
                    tryApplyShift(delta, events);
                }
            }
        }
        else if(tuning_.arr == sgl::Seconds{0.f})
        {
            while(tryApplyShift(delta, events))
            {
            }
        }
        else
        {
            arrTimer_ += dt;
            while(arrTimer_ >= tuning_.arr)
            {
                arrTimer_ -= tuning_.arr;
                if(!tryApplyShift(delta, events))
                {
                    break;
                }
            }
        }
    }

    prevLeftHeld_ = input.leftHeld;
    prevRightHeld_ = input.rightHeld;
}

void TetrisGame::handleDrops(const TetrisInput& input, sgl::Seconds dt, std::vector<TetrisEvent>& events)
{
    if(input.hardDrop && active_.has_value())
    {
        const Piece before = *active_;
        const Piece dropped = dropToFloor(grid_, before);
        const int rows = lowestRow(before) - lowestRow(dropped);
        active_ = dropped;
        noteLowest(*active_);
        applyHardDrop(score_, rows);
        events.push_back(HardDropped{});
        lockActive(events, true);
        return;
    }

    handleGravity(dt, input.softDropHeld, events);
}

void TetrisGame::handleGravity(sgl::Seconds dt, bool soft, std::vector<TetrisEvent>& events)
{
    if(!active_.has_value())
    {
        return;
    }

    const sgl::Seconds base = gravityInterval(score_.level);
    const sgl::Seconds interval = soft ? sgl::Seconds{base.count() / std::max(tuning_.softDropFactor, 1.f)} : base;

    fallTimer_ += dt;
    while(fallTimer_ >= interval && active_.has_value())
    {
        fallTimer_ -= interval;
        if(!tryApplyShift(Coord{0, -1}, events))
        {
            break;
        }
        if(soft)
        {
            applySoftDrop(score_, 1);
        }
    }
}

void TetrisGame::handleLock(sgl::Seconds dt, std::vector<TetrisEvent>& events)
{
    if(!active_.has_value())
    {
        return;
    }
    if(!grounded())
    {
        lockTimer_ = sgl::Seconds{0.f};
        return;
    }

    lockTimer_ += dt;
    if(lockTimer_ >= tuning_.lockDelay)
    {
        lockActive(events, false);
    }
}

void TetrisGame::lockActive(std::vector<TetrisEvent>& events, bool /*hardDropped*/)
{
    if(!active_.has_value())
    {
        return;
    }

    const Cells lockedCells = cells(*active_);
    grid_.lock(lockedCells, active_->type);
    events.push_back(PieceLocked{});
    active_.reset();
    holdAvailable_ = true;
    fallTimer_ = sgl::Seconds{0.f};
    lockTimer_ = sgl::Seconds{0.f};
    lockResets_ = 0;
    lowestRow_ = -1;
    resetDas();

    if(isLockOut(lockedCells))
    {
        over_ = true;
        events.push_back(GameOver{});
        return;
    }

    clearAndSpawn(events);
}

void TetrisGame::clearAndSpawn(std::vector<TetrisEvent>& events)
{
    const ClearedRows cleared = grid_.clearFullRows();
    if(cleared.count > 0)
    {
        const std::uint32_t levelBefore = score_.level;
        applyLineClear(score_, cleared.count);
        LinesCleared clearedEvent{.count = cleared.count};
        for(std::uint8_t i = 0; i < cleared.count; ++i)
        {
            clearedEvent.rows[i] = static_cast<std::uint8_t>(cleared.rows[i]);
        }
        events.push_back(clearedEvent);
        if(score_.level > levelBefore)
        {
            events.push_back(LevelUp{});
        }
    }

    spawnNext(events);
}

bool TetrisGame::spawnNext(std::vector<TetrisEvent>& events)
{
    return spawnType(bag_.next(), events);
}

bool TetrisGame::spawnType(PieceType type, std::vector<TetrisEvent>& events)
{
    const Piece piece = spawnPiece(type);
    if(!grid_.fits(cells(piece)))
    {
        over_ = true;
        events.push_back(GameOver{});
        return false;
    }

    active_ = piece;
    noteLowest(*active_);
    fallTimer_ = sgl::Seconds{0.f};
    lockTimer_ = sgl::Seconds{0.f};
    lockResets_ = 0;
    events.push_back(PieceSpawned{});
    return true;
}

} // namespace sgl::tetris
