#include <syndicate/app/Presentation.hpp>

#include <utility>

namespace sgl::syndicate
{

Presentation::Presentation(PawnAnimator& animator, std::uint64_t seed)
    : animator_{&animator}
    , dice_{seed}
{
}

void Presentation::onRolled(int roll)
{
    dice_.start(roll);
}

void Presentation::onPath(PlayerId id, const std::vector<sgl::Vec2f>& points)
{
    if(dice_.busy())
    {
        queued_ = QueuedPath{.id = id, .points = points};
        return;
    }
    animator_->start(id, points);
}

void Presentation::startQueuedPath()
{
    if(!queued_.has_value())
    {
        return;
    }
    QueuedPath path = std::move(*queued_);
    queued_.reset();
    animator_->start(path.id, path.points);
}

void Presentation::update(sgl::Seconds dt)
{
    if(dice_.busy())
    {
        dice_.update(dt);
        if(!dice_.busy())
        {
            startQueuedPath();
        }
        // Path motion starts on a later frame so a large dt that ends the tumble
        // does not also consume the whole pawn duration in one call.
        return;
    }
    if(queued_.has_value())
    {
        startQueuedPath();
    }
    animator_->update(dt);
}

bool Presentation::busy() const noexcept
{
    return dice_.busy() || queued_.has_value() || animator_->busy();
}

void Presentation::fastForward()
{
    dice_.fastForward();
    if(queued_.has_value())
    {
        startQueuedPath();
    }
    animator_->finishAll();
}

void Presentation::render(sgl::RenderQueue& queue, sgl::Vec2f diceCenter, const Theme& theme) const
{
    dice_.render(queue, diceCenter, theme);
}

} // namespace sgl::syndicate
