#include <algorithm>
#include <cmath>
#include <sgl/core/Rect.hpp>
#include <tappy/sim/TappyGame.hpp>

namespace sgl::tappy
{
namespace
{

[[nodiscard]] sgl::Rect<float> box(Vec2f pos, Vec2f size)
{
    return sgl::Rect<float>{pos, size};
}

} // namespace

TappyGame::TappyGame(std::uint64_t seed, Tuning tuning)
    : tuning_{tuning}
    , rng_{seed}
{
    player_.size = Vec2f{tuning_.playerW, tuning_.playerH};
    player_.pos = Vec2f{tuning_.playerSpawnX, tuning_.playerSpawnY};
    player_.vel = {};
    nextGateX_ = tuning_.playerSpawnX + tuning_.gateSpacing;
    distance_ = player_.pos.x;
    ensureGates();
}

std::vector<TappyEvent> TappyGame::step(const SimInput& input, sgl::Seconds dt)
{
    if(over_)
    {
        return {};
    }

    const float seconds = std::max(dt.count(), 0.f);
    std::vector<TappyEvent> events;

    if(input.flap)
    {
        player_.vel.y = tuning_.flapSpeed;
        events.push_back(Flapped{});
    }

    player_.vel.y -= tuning_.gravity * seconds;
    player_.vel.y = std::clamp(player_.vel.y, -tuning_.maxFall, tuning_.maxRise);
    player_.pos.y += player_.vel.y * seconds;

    const float speed = scrollSpeedAt(distance_);
    player_.pos.x += speed * seconds;
    distance_ = std::max(distance_, player_.pos.x);

    ensureGates();
    cullGates();
    clearGates(events);
    collide(events);

    if(!over_)
    {
        if(player_.pos.y < 0.f || player_.pos.y + player_.size.y > kWorldH)
        {
            die(events);
        }
    }

    return events;
}

PlayerView TappyGame::player() const noexcept
{
    return player_;
}

const std::vector<GateView>& TappyGame::gates() const noexcept
{
    return gates_;
}

float TappyGame::cameraLeft() const noexcept
{
    return player_.pos.x - kCameraFollowFrac * kWorldW;
}

int TappyGame::score() const noexcept
{
    return score_;
}

bool TappyGame::over() const noexcept
{
    return over_;
}

float TappyGame::scrollSpeed() const noexcept
{
    return scrollSpeedAt(distance_);
}

float TappyGame::distance() const noexcept
{
    return distance_;
}

float TappyGame::gapAt(float worldX) const noexcept
{
    return std::max(tuning_.gapMin, tuning_.gapStart - tuning_.gapShrink * worldX);
}

float TappyGame::scrollSpeedAt(float worldX) const noexcept
{
    return std::min(tuning_.scrollCap, tuning_.scrollBase + tuning_.scrollAccel * worldX);
}

void TappyGame::ensureGates()
{
    if(tuning_.gateSpacing <= 0.f || tuning_.gateWidth <= 0.f)
    {
        return;
    }

    const float viewRight = cameraLeft() + kWorldW;
    const float limit = viewRight + tuning_.gateSpacing * 2.f;
    while(nextGateX_ <= limit)
    {
        const float gap = gapAt(nextGateX_);
        const float halfGap = gap * 0.5f;
        const float minCenter = halfGap + tuning_.gapCenterMargin;
        const float maxCenter = kWorldH - halfGap - tuning_.gapCenterMargin;
        float center = kWorldH * 0.5f;
        if(maxCenter > minCenter)
        {
            center = rng_.uniformFloat(minCenter, maxCenter);
        }

        const float bottomH = std::max(0.f, center - halfGap);
        const float topY = center + halfGap;
        const float topH = std::max(0.f, kWorldH - topY);

        gates_.push_back(GateView{
            .topPos = {nextGateX_, topY},
            .topSize = {tuning_.gateWidth, topH},
            .bottomPos = {nextGateX_, 0.f},
            .bottomSize = {tuning_.gateWidth, bottomH},
            .cleared = false,
        });
        nextGateX_ += tuning_.gateSpacing;
    }
}

void TappyGame::cullGates()
{
    const float cutoff = cameraLeft() - tuning_.gateWidth;
    std::erase_if(gates_, [cutoff](const GateView& gate) {
        return gate.topPos.x + gate.topSize.x < cutoff;
    });
}

void TappyGame::clearGates(std::vector<TappyEvent>& events)
{
    const float playerRight = player_.pos.x + player_.size.x;
    for(GateView& gate: gates_)
    {
        if(gate.cleared)
        {
            continue;
        }
        const float gateRight = gate.topPos.x + gate.topSize.x;
        if(playerRight > gateRight)
        {
            gate.cleared = true;
            ++score_;
            events.push_back(GateCleared{.gateIndex = nextGateIndex_++});
        }
    }
}

void TappyGame::collide(std::vector<TappyEvent>& events)
{
    const sgl::Rect<float> playerBox = box(player_.pos, player_.size);
    for(const GateView& gate: gates_)
    {
        if(playerBox.intersects(box(gate.topPos, gate.topSize)) ||
           playerBox.intersects(box(gate.bottomPos, gate.bottomSize)))
        {
            die(events);
            return;
        }
    }
}

void TappyGame::die(std::vector<TappyEvent>& events)
{
    if(over_)
    {
        return;
    }
    over_ = true;
    events.push_back(Died{});
}

} // namespace sgl::tappy
