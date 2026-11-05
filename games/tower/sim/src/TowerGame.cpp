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
    timeSinceLand_ += seconds;

    std::vector<TowerEvent> events;
    const bool wasGrounded = player_.grounded;
    if(input.jump && wasGrounded)
    {
        float jump = jumpSpeedEffective();
        if(const PlatformView* support = supportPlatform())
        {
            const float feetCenter = player_.pos.x + player_.size.x * 0.5f;
            const float left = support->pos.x;
            const float right = support->pos.x + support->size.x;
            const bool nearEdge = feetCenter - left <= tuning_.edgeJumpMargin || right - feetCenter <= tuning_.edgeJumpMargin;
            if(nearEdge)
            {
                jump *= tuning_.edgeJumpMul;
                events.push_back(EdgeJumped{});
            }
        }
        player_.vel.y = jump;
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
        onDied(events);
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

int TowerGame::combo() const noexcept
{
    return combo_;
}

bool TowerGame::over() const noexcept
{
    return over_;
}

float TowerGame::playLeft() const noexcept
{
    return std::max(0.f, tuning_.wallThickness);
}

float TowerGame::playRight() const noexcept
{
    return std::max(playLeft(), kWorldW - std::max(0.f, tuning_.wallThickness));
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
    if(combo_ > 1 && tuning_.comboJumpScale > 0.f && tuning_.comboCap > 1)
    {
        const int ranks = std::min(combo_ - 1, tuning_.comboCap - 1);
        jump *= 1.f + tuning_.comboJumpScale * static_cast<float>(ranks);
    }
    return jump;
}

void TowerGame::spawnStart()
{
    const float left = playLeft();
    const float right = playRight();
    const float shaft = std::max(0.f, right - left);
    const float width = std::clamp(tuning_.startPlatformW, 0.f, shaft);
    platforms_.push_back(PlatformView{
        Vec2f{(left + right - width) * 0.5f, -tuning_.platformH},
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
    const float left = playLeft();
    const float right = playRight();
    const float shaft = std::max(0.f, right - left);
    const float width = std::clamp(widthAt(surfaceY), 0.f, shaft);
    const float minX = left;
    const float maxX = std::max(minX, right - width);

    float firstX = (band % 2 == 0) ? minX : maxX;
    if(maxX > minX)
    {
        const float jitter = std::min(40.f, maxX - minX);
        firstX = std::clamp(firstX + rng_.uniformFloat(-jitter * 0.25f, jitter * 0.25f), minX, maxX);
    }

    PlatformView first{
        Vec2f{firstX, surfaceY - tuning_.platformH},
        Vec2f{width, tuning_.platformH},
    };
    maybeSpawnBonus(first);
    platforms_.push_back(first);

    if(kPlatformsPerBand < 2 || maxX <= minX)
    {
        return;
    }

    const float firstCenter = firstX + width * 0.5f;
    const float gap = std::max(0.f, tuning_.maxSiblingGap);
    const float secondMinCenter = firstCenter - gap;
    const float secondMaxCenter = firstCenter + gap;
    const float secondMinX = std::clamp(secondMinCenter - width * 0.5f, minX, maxX);
    const float secondMaxX = std::clamp(secondMaxCenter - width * 0.5f, minX, maxX);
    const float lo = std::min(secondMinX, secondMaxX);
    const float hi = std::max(secondMinX, secondMaxX);
    const float secondX = rng_.uniformFloat(lo, hi);

    PlatformView second{
        Vec2f{secondX, surfaceY - tuning_.platformH},
        Vec2f{width, tuning_.platformH},
    };
    maybeSpawnBonus(second);
    platforms_.push_back(second);
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
    const float left = playLeft();
    const float right = playRight();

    WallSide hit = WallSide::None;
    if(player_.pos.x < left)
    {
        player_.pos.x = left;
        hit = WallSide::Left;
    }
    else if(player_.pos.x + player_.size.x > right)
    {
        player_.pos.x = right - player_.size.x;
        hit = WallSide::Right;
    }
    else
    {
        return;
    }

    const bool approaching =
        (hit == WallSide::Left && player_.vel.x < 0.f) || (hit == WallSide::Right && player_.vel.x > 0.f);
    if(!approaching)
    {
        // Pressed into the wall after a bounce: keep them inside the shaft, no kick spam.
        if(hit == WallSide::Left)
        {
            player_.vel.x = std::max(0.f, player_.vel.x);
        }
        else
        {
            player_.vel.x = std::min(0.f, player_.vel.x);
        }
        return;
    }

    if(hit == WallSide::Left)
    {
        player_.vel.x = std::abs(player_.vel.x) * tuning_.wallBounce;
    }
    else
    {
        player_.vel.x = -std::abs(player_.vel.x) * tuning_.wallBounce;
    }

    // Climb boost only when alternating walls (or first wall after a land). Same-wall
    // re-hits while holding into one side must not keep launching the player upward.
    const bool climbBoost = !player_.grounded && lastBoostWall_ != hit;
    if(climbBoost)
    {
        float boost = tuning_.wallKickBase + tuning_.wallKickSpeedMul * std::abs(player_.vel.x);
        if(combo_ > 0 && tuning_.wallComboMul > 0.f)
        {
            const int ranks = std::min(combo_, tuning_.comboCap);
            boost *= 1.f + tuning_.wallComboMul * static_cast<float>(ranks);
        }
        player_.vel.y = std::max(player_.vel.y, boost);
        timeSinceLand_ = 0.f;
        lastBoostWall_ = hit;
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

void TowerGame::onDied(std::vector<TowerEvent>& events)
{
    over_ = true;
    combo_ = 0;
    lastBoostWall_ = WallSide::None;
    events.push_back(Died{});
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
    lastBoostWall_ = WallSide::None;
    if(timeSinceLand_ <= tuning_.comboWindow)
    {
        combo_ = std::min(combo_ + 1, std::max(tuning_.comboCap, 1));
    }
    else
    {
        combo_ = 1;
    }
    score_ += combo_ * tuning_.comboPoints;
    timeSinceLand_ = 0.f;
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
    return supportPlatform() != nullptr;
}

const PlatformView* TowerGame::supportPlatform() const noexcept
{
    for(const PlatformView& platform: platforms_)
    {
        const float top = platform.pos.y + platform.size.y;
        if(std::abs(player_.pos.y - top) <= kSurfaceEps && overlapsX(platform))
        {
            return &platform;
        }
    }
    return nullptr;
}

} // namespace sgl::tower
