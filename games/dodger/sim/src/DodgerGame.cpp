#include <algorithm>
#include <cmath>
#include <dodger/sim/DodgerGame.hpp>
#include <sgl/core/Rect.hpp>
#include <utility>

namespace sgl::dodger
{

DodgerGame::DodgerGame(std::uint64_t seed, Tuning tuning)
    : tuning_{tuning}
    , rng_{seed}
{
    paddle_.size = Vec2f{tuning_.paddleW, tuning_.paddleH};
    paddle_.pos = Vec2f{(kWorldW - tuning_.paddleW) * 0.5f, kPaddleY};
}

std::vector<DodgerEvent> DodgerGame::step(const SimInput& input, sgl::Seconds dt)
{
    std::vector<DodgerEvent> events;
    if(over_)
    {
        return events;
    }

    const float seconds = dt.count();
    float direction = 0.f;
    if(input.left != input.right)
    {
        direction = input.right ? 1.f : -1.f;
    }
    paddle_.pos.x += direction * tuning_.paddleSpeed * seconds;
    paddle_.pos.x = std::clamp(paddle_.pos.x, 0.f, kWorldW - tuning_.paddleW);
    paddle_.pos.y = kPaddleY;

    if(tuning_.spawnInterval > 0.f)
    {
        spawnAccum_ += seconds;
        while(spawnAccum_ >= tuning_.spawnInterval)
        {
            spawnAccum_ -= tuning_.spawnInterval;
            const auto columns = static_cast<std::uint32_t>(std::floor(kWorldW / tuning_.hazardW));
            const float x = static_cast<float>(rng_.uniformBelow(columns)) * tuning_.hazardW;
            hazards_.push_back(HazardView{
                Vec2f{x, -tuning_.hazardH},
                Vec2f{tuning_.hazardW, tuning_.hazardH},
            });
        }
    }

    std::vector<HazardView> stillFalling;
    stillFalling.reserve(hazards_.size());
    for(HazardView& hazard: hazards_)
    {
        hazard.pos.y += tuning_.hazardSpeed * seconds;
        if(hazard.pos.y > kWorldH)
        {
            ++score_;
            events.push_back(HazardCleared{});
        }
        else
        {
            stillFalling.push_back(hazard);
        }
    }
    hazards_ = std::move(stillFalling);

    const sgl::Rect<float> paddleRect{paddle_.pos, paddle_.size};
    for(const HazardView& hazard: hazards_)
    {
        if(paddleRect.intersects(sgl::Rect<float>{hazard.pos, hazard.size}))
        {
            over_ = true;
            events.push_back(Died{});
            break;
        }
    }

    return events;
}

PaddleView DodgerGame::paddle() const noexcept
{
    return paddle_;
}

const std::vector<HazardView>& DodgerGame::hazards() const noexcept
{
    return hazards_;
}

int DodgerGame::score() const noexcept
{
    return score_;
}

bool DodgerGame::over() const noexcept
{
    return over_;
}

} // namespace sgl::dodger
