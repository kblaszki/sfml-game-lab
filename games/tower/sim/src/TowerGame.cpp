#include <algorithm>
#include <cmath>
#include <sgl/core/Rect.hpp>
#include <tower/sim/TowerGame.hpp>

namespace sgl::tower
{
namespace
{

constexpr float kWidthShrinkBands = 8.f;
constexpr float kSurfaceEps = 0.001f;
constexpr int kPlatformsPerBand = 2;

[[nodiscard]] sgl::Rect<float> box(Vec2f pos, Vec2f size)
{
    return sgl::Rect<float>{pos, size};
}

} // namespace

TowerGame::TowerGame(std::uint64_t seed, Tuning tuning)
    : tuning_{tuning}
    , rng_{seed}
{
    player_.size = Vec2f{tuning_.playerW, tuning_.playerH};
    player_.pos = Vec2f{(kWorldW - tuning_.playerW) * 0.5f, 0.f};
    player_.vel = {};
    player_.grounded = true;
    cameraBottom_ = player_.pos.y - kViewH * 0.65f;
    peakFeet_ = player_.pos.y;
    score_ = static_cast<int>(std::floor(peakFeet_));
    spawnStart();
    ensurePlatforms();
}

std::vector<TowerEvent> TowerGame::step(const SimInput& input, sgl::Seconds dt)
{
    if(over_)
    {
        return {};
    }

    const float seconds = std::max(dt.count(), 0.f);
    tickBoosts(seconds);

    const bool wasGrounded = player_.grounded;
    if(input.jump && wasGrounded)
    {
        player_.vel.y = jumpSpeedEffective();
        player_.grounded = false;
    }

    const float speedCap = runSpeedCap();
    float accel = 0.f;
    if(input.left != input.right)
    {
        accel = input.right ? tuning_.moveAccel : -tuning_.moveAccel;
    }
    player_.vel.x += accel * seconds;
    player_.vel.x = std::clamp(player_.vel.x, -speedCap, speedCap);
    if(wasGrounded)
    {
        player_.vel.x *= std::exp(-tuning_.iceFriction * seconds);
    }

    const float previousFeet = player_.pos.y;
    player_.pos.x += player_.vel.x * seconds;

    std::vector<TowerEvent> events;
    bounceWalls(events);

    if(player_.grounded && !supported())
    {
        player_.grounded = false;
    }

    if(player_.grounded)
    {
        player_.vel.y = 0.f;
    }
    else
    {
        player_.vel.y -= tuning_.gravity * seconds;
    }
    player_.pos.y += player_.vel.y * seconds;

    tryLand(previousFeet, events);
    collectBonuses(events);
    notePeak();

    const float follow = player_.pos.y - kViewH * 0.65f;
    cameraBottom_ = std::max(cameraBottom_, follow);
    ensurePlatforms();
    cullPlatforms();
    cullBonuses();

    const float playerTop = player_.pos.y + player_.size.y;
    if(playerTop < cameraBottom_ - tuning_.deathMargin)
    {
        over_ = true;
        events.push_back(Died{});
    }
    return events;
}

PlayerView TowerGame::player() const noexcept
{
    return player_;
}

const std::vector<PlatformView>& TowerGame::platforms() const noexcept
{
    return platforms_;
}

const std::vector<BonusView>& TowerGame::bonuses() const noexcept
{
    return bonuses_;
}

float TowerGame::cameraBottom() const noexcept
{
    return cameraBottom_;
}

int TowerGame::score() const noexcept
{
    return score_;
}

bool TowerGame::over() const noexcept
{
    return over_;
}

float TowerGame::heightScale() const noexcept
{
    const float extra = std::min(tuning_.heightAccelCap, tuning_.heightAccel * peakFeet_);
    return 1.f + extra;
}

float TowerGame::runSpeedCap() const noexcept
{
    float cap = tuning_.maxRunSpeed * heightScale();
    if(speedBoostLeft_ > 0.f)
    {
        cap *= tuning_.speedBoostMul;
    }
    return cap;
}

float TowerGame::jumpSpeedEffective() const noexcept
{
    float jump = tuning_.jumpSpeed * heightScale();
    if(jumpBoostLeft_ > 0.f)
    {
        jump *= tuning_.jumpBoostMul;
    }
    return jump;
}

void TowerGame::spawnStart()
{
    const float width = std::clamp(tuning_.startPlatformW, 0.f, kWorldW);
    platforms_.push_back(PlatformView{
        Vec2f{(kWorldW - width) * 0.5f, -tuning_.platformH},
        Vec2f{width, tuning_.platformH},
    });
}

void TowerGame::maybeSpawnBonus(const PlatformView& platform)
{
    if(tuning_.bonusChance <= 0.f || tuning_.bonusSize <= 0.f)
    {
        return;
    }
    if(rng_.uniformFloat(0.f, 1.f) >= tuning_.bonusChance)
    {
        return;
    }

    const BonusKind kind = static_cast<BonusKind>(rng_.uniformBelow(3));
    const float size = tuning_.bonusSize;
    const float surfaceY = platform.pos.y + platform.size.y;
    const float x = platform.pos.x + (platform.size.x - size) * 0.5f;
    const float y = surfaceY + tuning_.playerH * 0.3f;
    bonuses_.push_back(BonusView{
        .pos = {x, y},
        .size = {size, size},
        .kind = kind,
    });
}

void TowerGame::spawnBand(int band)
{
    const float surfaceY = static_cast<float>(band) * tuning_.bandHeight;
    const float width = std::clamp(widthAt(surfaceY), 0.f, kWorldW);
    const float maxX = std::max(0.f, kWorldW - width);
    for(int slot = 0; slot < kPlatformsPerBand; ++slot)
    {
        PlatformView platform{
            Vec2f{rng_.uniformFloat(0.f, maxX), surfaceY - tuning_.platformH},
            Vec2f{width, tuning_.platformH},
        };
        maybeSpawnBonus(platform);
        platforms_.push_back(platform);
    }
}

void TowerGame::ensurePlatforms()
{
    if(tuning_.bandHeight <= 0.f)
    {
        return;
    }
    const float viewTop = cameraBottom_ + kViewH;
    const float limit = std::max(viewTop, player_.pos.y) + tuning_.bandHeight * 2.f;
    while(static_cast<float>(nextBand_) * tuning_.bandHeight <= limit)
    {
        spawnBand(nextBand_);
        ++nextBand_;
    }
}

void TowerGame::cullPlatforms()
{
    const float cutoff = cameraBottom_ - kViewH;
    std::erase_if(platforms_,
                  [cutoff](const PlatformView& platform) { return platform.pos.y + platform.size.y < cutoff; });
}

void TowerGame::cullBonuses()
{
    const float cutoff = cameraBottom_ - kViewH;
    std::erase_if(bonuses_, [cutoff](const BonusView& bonus) { return bonus.pos.y + bonus.size.y < cutoff; });
}

void TowerGame::notePeak() noexcept
{
    peakFeet_ = std::max(peakFeet_, player_.pos.y);
    score_ = std::max(score_, static_cast<int>(std::floor(peakFeet_)));
}

void TowerGame::bounceWalls(std::vector<TowerEvent>& events)
{
    bool bounced = false;
    if(player_.pos.x < 0.f)
    {
        player_.pos.x = 0.f;
        player_.vel.x = std::abs(player_.vel.x) * tuning_.wallBounce;
        bounced = true;
    }
    else if(player_.pos.x + player_.size.x > kWorldW)
    {
        player_.pos.x = kWorldW - player_.size.x;
        player_.vel.x = -std::abs(player_.vel.x) * tuning_.wallBounce;
        bounced = true;
    }

    if(!bounced)
    {
        return;
    }
    if(player_.vel.y < 0.f)
    {
        player_.vel.y = std::max(player_.vel.y, tuning_.wallKick);
    }
    events.push_back(WallBounced{});
}

void TowerGame::collectBonuses(std::vector<TowerEvent>& events)
{
    const sgl::Rect<float> playerBox = box(player_.pos, player_.size);
    for(std::size_t i = 0; i < bonuses_.size();)
    {
        if(!playerBox.intersects(box(bonuses_[i].pos, bonuses_[i].size)))
        {
            ++i;
            continue;
        }

        const BonusKind kind = bonuses_[i].kind;
        int points = 0;
        switch(kind)
        {
        case BonusKind::Speed:
            speedBoostLeft_ = tuning_.boostDuration;
            break;
        case BonusKind::Jump:
            jumpBoostLeft_ = tuning_.boostDuration;
            break;
        case BonusKind::Points:
            points = tuning_.pointsBonus;
            score_ += points;
            break;
        }
        events.push_back(BonusPicked{.kind = kind, .points = points});
        bonuses_.erase(bonuses_.begin() + static_cast<std::ptrdiff_t>(i));
    }
}

void TowerGame::tickBoosts(float seconds) noexcept
{
    speedBoostLeft_ = std::max(0.f, speedBoostLeft_ - seconds);
    jumpBoostLeft_ = std::max(0.f, jumpBoostLeft_ - seconds);
}

void TowerGame::tryLand(float previousFeet, std::vector<TowerEvent>& events)
{
    if(player_.grounded || player_.vel.y > 0.f)
    {
        return;
    }

    bool found = false;
    float landedTop = 0.f;
    for(const PlatformView& platform: platforms_)
    {
        const float top = platform.pos.y + platform.size.y;
        if(previousFeet + kSurfaceEps < top || player_.pos.y > top + kSurfaceEps || !overlapsX(platform))
        {
            continue;
        }
        if(!found || top > landedTop)
        {
            found = true;
            landedTop = top;
        }
    }

    if(!found)
    {
        return;
    }
    player_.pos.y = landedTop;
    player_.vel.y = 0.f;
    player_.grounded = true;
    events.push_back(Landed{});
}

float TowerGame::widthAt(float surfaceY) const noexcept
{
    const float scale = std::max(tuning_.bandHeight, 1.f) * kWidthShrinkBands;
    const float height = std::max(surfaceY, 0.f);
    const float span = tuning_.startPlatformW - tuning_.minPlatformW;
    return tuning_.minPlatformW + span * std::exp(-height / scale);
}

bool TowerGame::overlapsX(const PlatformView& platform) const noexcept
{
    const sgl::Rect<float> playerBox{Vec2f{player_.pos.x, 0.f}, Vec2f{player_.size.x, 1.f}};
    const sgl::Rect<float> platformBox{Vec2f{platform.pos.x, 0.f}, Vec2f{platform.size.x, 1.f}};
    return playerBox.intersects(platformBox);
}

bool TowerGame::supported() const noexcept
{
    for(const PlatformView& platform: platforms_)
    {
        const float top = platform.pos.y + platform.size.y;
        if(std::abs(player_.pos.y - top) <= kSurfaceEps && overlapsX(platform))
        {
            return true;
        }
    }
    return false;
}

} // namespace sgl::tower
