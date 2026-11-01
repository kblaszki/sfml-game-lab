#include <asteroids/sim/AsteroidsGame.hpp>
#include <cmath>
#include <numbers>
#include <utility>

namespace sgl::asteroids
{
namespace
{

constexpr float kPi = std::numbers::pi_v<float>;
constexpr float kBulletRadius = 3.f;
constexpr float kSpawnGap = 180.f;
constexpr float kSpawnClearance = 64.f;
constexpr float kDriftMin = 18.f;
constexpr float kDriftMax = 34.f;
constexpr float kMediumChildSpeed = 48.f;
constexpr float kSmallChildSpeed = 64.f;
constexpr float kSpreadMin = 0.22f;
constexpr float kSpreadMax = 0.48f;

[[nodiscard]] Vec2f directionOf(float heading) noexcept
{
    return Vec2f{std::cos(heading), std::sin(heading)};
}

[[nodiscard]] float wrapCoord(float value, float limit) noexcept
{
    float wrapped = std::fmod(value, limit);
    if(wrapped < 0.f)
    {
        wrapped += limit;
    }
    if(wrapped >= limit)
    {
        wrapped -= limit;
    }
    return wrapped;
}

[[nodiscard]] Vec2f wrapPosition(Vec2f pos) noexcept
{
    pos.x = wrapCoord(pos.x, kWorldW);
    pos.y = wrapCoord(pos.y, kWorldH);
    return pos;
}

[[nodiscard]] float wrapDelta(float from, float to, float limit) noexcept
{
    float delta = from - to;
    const float half = limit * 0.5f;
    if(delta > half)
    {
        delta -= limit;
    }
    else if(delta <= -half)
    {
        delta += limit;
    }
    return delta;
}

[[nodiscard]] bool circlesOverlap(Vec2f a, float aRadius, Vec2f b, float bRadius) noexcept
{
    const float dx = wrapDelta(a.x, b.x, kWorldW);
    const float dy = wrapDelta(a.y, b.y, kWorldH);
    const float reach = aRadius + bRadius;
    return dx * dx + dy * dy <= reach * reach;
}

} // namespace

AsteroidsGame::AsteroidsGame(std::uint64_t seed, Tuning tuning)
    : tuning_{tuning}
    , rng_{seed}
{
    ship_.pos = Vec2f{kWorldW * 0.5f, kWorldH * 0.5f};
    spawnWave();
    rebuildViews();
}

std::vector<AsteroidsEvent> AsteroidsGame::step(const SimInput& input, sgl::Seconds dt)
{
    std::vector<AsteroidsEvent> events;
    if(over_)
    {
        return events;
    }

    float seconds = dt.count();
    if(seconds < 0.f)
    {
        seconds = 0.f;
    }

    turnShip(input, seconds);
    thrustShip(input.thrust, seconds);
    updateFire(input, seconds, events);
    advanceBullets(seconds);
    advanceRocks(seconds);

    const bool hadRocks = !rocks_.empty();
    hitRocks(events);
    if(!over_)
    {
        hitShip(events);
    }
    if(!over_ && hadRocks && rocks_.empty())
    {
        events.push_back(WaveCleared{});
        spawnWave();
    }
    rebuildViews();
    return events;
}

ShipView AsteroidsGame::ship() const noexcept
{
    return ShipView{ship_.pos, ship_.vel, ship_.heading, tuning_.shipRadius};
}

const std::vector<BulletView>& AsteroidsGame::bullets() const noexcept
{
    return bulletViews_;
}

const std::vector<RockView>& AsteroidsGame::rocks() const noexcept
{
    return rockViews_;
}

int AsteroidsGame::score() const noexcept
{
    return score_;
}

bool AsteroidsGame::over() const noexcept
{
    return over_;
}

void AsteroidsGame::spawnWave()
{
    rocks_.clear();
    const int count = tuning_.initialLargeRocks;
    if(count <= 0)
    {
        return;
    }

    const float ring = tuning_.shipRadius + tuning_.largeRadius + kSpawnGap;
    const float minDist = tuning_.shipRadius + tuning_.largeRadius + kSpawnClearance;
    const float minDistSq = minDist * minDist;
    const float countF = static_cast<float>(count);

    for(int i = 0; i < count; ++i)
    {
        float angle = static_cast<float>(i) * (2.f * kPi) / countF;
        Vec2f pos = wrapPosition(ship_.pos + directionOf(angle) * ring);
        const float dx = wrapDelta(pos.x, ship_.pos.x, kWorldW);
        const float dy = wrapDelta(pos.y, ship_.pos.y, kWorldH);
        if(dx * dx + dy * dy < minDistSq)
        {
            bool placed = false;
            for(int attempt = 0; attempt < 8; ++attempt)
            {
                angle = rng_.uniformFloat(0.f, 2.f * kPi);
                pos = wrapPosition(ship_.pos + directionOf(angle) * ring);
                const float tryDx = wrapDelta(pos.x, ship_.pos.x, kWorldW);
                const float tryDy = wrapDelta(pos.y, ship_.pos.y, kWorldH);
                if(tryDx * tryDx + tryDy * tryDy >= minDistSq)
                {
                    placed = true;
                    break;
                }
            }
            if(!placed)
            {
                pos = wrapPosition(Vec2f{ship_.pos.x + kWorldW * 0.5f, ship_.pos.y + kWorldH * 0.5f});
                angle = 0.f;
            }
        }

        const float drift = rng_.uniformFloat(kDriftMin, kDriftMax);
        rocks_.push_back(Rock{pos, directionOf(angle) * drift, RockSize::Large});
    }
}

void AsteroidsGame::turnShip(const SimInput& input, float seconds)
{
    float delta = 0.f;
    if(input.rotateCcw)
    {
        delta += tuning_.rotateSpeed;
    }
    if(input.rotateCw)
    {
        delta -= tuning_.rotateSpeed;
    }
    ship_.heading += delta * seconds;
}

void AsteroidsGame::thrustShip(bool thrust, float seconds)
{
    if(thrust)
    {
        ship_.vel += directionOf(ship_.heading) * (tuning_.thrustAccel * seconds);
        const float maxSpeed = tuning_.maxShipSpeed;
        const float speedSq = sgl::lengthSq(ship_.vel);
        if(maxSpeed <= 0.f)
        {
            ship_.vel = {};
        }
        else if(speedSq > maxSpeed * maxSpeed)
        {
            ship_.vel = ship_.vel * (maxSpeed / std::sqrt(speedSq));
        }
    }
    ship_.pos += ship_.vel * seconds;
    ship_.pos = wrapPosition(ship_.pos);
}

void AsteroidsGame::updateFire(const SimInput& input, float seconds, std::vector<AsteroidsEvent>& events)
{
    fireCooldownLeft_ -= seconds;
    if(fireCooldownLeft_ < 0.f)
    {
        fireCooldownLeft_ = 0.f;
    }
    if(!input.fire || fireCooldownLeft_ > 0.f)
    {
        return;
    }

    const Vec2f direction = directionOf(ship_.heading);
    bullets_.push_back(Bullet{ship_.pos, direction * tuning_.bulletSpeed, 0.f});
    fireCooldownLeft_ = tuning_.fireCooldown;
    events.push_back(ShotFired{});
}

void AsteroidsGame::advanceBullets(float seconds)
{
    for(Bullet& bullet: bullets_)
    {
        bullet.age += seconds;
        bullet.pos += bullet.vel * seconds;
        bullet.pos = wrapPosition(bullet.pos);
    }
}

void AsteroidsGame::advanceRocks(float seconds)
{
    for(Rock& rock: rocks_)
    {
        rock.pos += rock.vel * seconds;
        rock.pos = wrapPosition(rock.pos);
    }
}

void AsteroidsGame::hitRocks(std::vector<AsteroidsEvent>& events)
{
    std::vector<char> gone(rocks_.size(), 0);
    std::vector<Rock> spawned;
    std::vector<Bullet> kept;
    kept.reserve(bullets_.size());

    for(const Bullet& bullet: bullets_)
    {
        bool consumed = false;
        for(std::size_t index = 0; index < rocks_.size(); ++index)
        {
            if(gone[index] != 0)
            {
                continue;
            }
            const Rock& rock = rocks_[index];
            if(!circlesOverlap(bullet.pos, kBulletRadius, rock.pos, radiusOf(rock.size)))
            {
                continue;
            }
            gone[index] = 1;
            consumed = true;
            const int gained = scoreOf(rock.size);
            score_ += gained;
            events.push_back(RockHit{rock.size, gained});
            spawnChildren(rock, spawned);
            break;
        }
        if(!consumed && bullet.age < tuning_.bulletLifetime)
        {
            kept.push_back(bullet);
        }
    }

    std::vector<Rock> next;
    next.reserve(rocks_.size() + spawned.size());
    for(std::size_t index = 0; index < rocks_.size(); ++index)
    {
        if(gone[index] == 0)
        {
            next.push_back(rocks_[index]);
        }
    }
    next.insert(next.end(), spawned.begin(), spawned.end());
    rocks_ = std::move(next);
    bullets_ = std::move(kept);
}

void AsteroidsGame::hitShip(std::vector<AsteroidsEvent>& events)
{
    for(const Rock& rock: rocks_)
    {
        if(circlesOverlap(ship_.pos, tuning_.shipRadius, rock.pos, radiusOf(rock.size)))
        {
            over_ = true;
            events.push_back(Died{});
            return;
        }
    }
}

void AsteroidsGame::spawnChildren(const Rock& parent, std::vector<Rock>& out)
{
    if(parent.size == RockSize::Small)
    {
        return;
    }

    const RockSize next = (parent.size == RockSize::Large) ? RockSize::Medium : RockSize::Small;
    const float baseSpeed = (next == RockSize::Medium) ? kMediumChildSpeed : kSmallChildSpeed;
    const float speedSq = sgl::lengthSq(parent.vel);
    const float baseAngle = (speedSq > 0.f) ? std::atan2(parent.vel.y, parent.vel.x) : 0.f;
    const float spreadA = rng_.uniformFloat(kSpreadMin, kSpreadMax);
    const float spreadB = rng_.uniformFloat(kSpreadMin, kSpreadMax);
    const float speedA = baseSpeed * rng_.uniformFloat(0.9f, 1.1f);
    const float speedB = baseSpeed * rng_.uniformFloat(0.9f, 1.1f);
    const Vec2f pos = wrapPosition(parent.pos);
    out.push_back(Rock{pos, directionOf(baseAngle + spreadA) * speedA, next});
    out.push_back(Rock{pos, directionOf(baseAngle - spreadB) * speedB, next});
}

void AsteroidsGame::rebuildViews()
{
    bulletViews_.clear();
    bulletViews_.reserve(bullets_.size());
    for(const Bullet& bullet: bullets_)
    {
        bulletViews_.push_back(BulletView{bullet.pos, kBulletRadius});
    }

    rockViews_.clear();
    rockViews_.reserve(rocks_.size());
    for(const Rock& rock: rocks_)
    {
        rockViews_.push_back(RockView{rock.pos, rock.size, radiusOf(rock.size)});
    }
}

float AsteroidsGame::radiusOf(RockSize size) const noexcept
{
    switch(size)
    {
        case RockSize::Large:
            return tuning_.largeRadius;
        case RockSize::Medium:
            return tuning_.mediumRadius;
        case RockSize::Small:
            return tuning_.smallRadius;
    }
    return tuning_.largeRadius;
}

int AsteroidsGame::scoreOf(RockSize size) const noexcept
{
    switch(size)
    {
        case RockSize::Large:
            return tuning_.largeScore;
        case RockSize::Medium:
            return tuning_.mediumScore;
        case RockSize::Small:
            return tuning_.smallScore;
    }
    return 0;
}

} // namespace sgl::asteroids
