#include <asteroids/app/InputMapping.hpp>
#include <asteroids/app/PlayScene.hpp>
#include <asteroids/app/Theme.hpp>
#include <asteroids/sim/Events.hpp>
#include <asteroids/sim/Types.hpp>
#include <cmath>
#include <numbers>
#include <sgl/core/Color.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <string>
#include <string_view>
#include <variant>

namespace sgl::asteroids
{
namespace
{

constexpr sgl::Color kWhite{255, 255, 255, 255};

[[nodiscard]] sgl::Vec2f toScreen(Vec2f sim)
{
    return {sim.x, kWorldH - sim.y};
}

// Kenney ships point up (screen -Y). Sim heading 0 is +X; positive heading is CCW toward +Y.
// With Y-flipped drawing, map heading to SFML clockwise degrees from sprite-up.
[[nodiscard]] float shipRotationDeg(float headingRad)
{
    return 90.f - headingRad * (180.f / std::numbers::pi_v<float>);
}

[[nodiscard]] sgl::MeshCmd shipMesh(const ShipView& ship)
{
    const float c = std::cos(ship.heading);
    const float s = std::sin(ship.heading);
    const Vec2f forward{c, s};
    const Vec2f side{-s, c};
    const Vec2f tip = ship.pos + forward * (ship.radius * 1.6f);
    const Vec2f left = ship.pos - forward * (ship.radius * 0.9f) + side * (ship.radius * 0.95f);
    const Vec2f right = ship.pos - forward * (ship.radius * 0.9f) - side * (ship.radius * 0.95f);
    return sgl::MeshCmd{
        .vertices =
            {
                sgl::Vertex{.position = toScreen(tip), .color = shipColor},
                sgl::Vertex{.position = toScreen(right), .color = shipColor},
                sgl::Vertex{.position = toScreen(left), .color = shipColor},
            },
    };
}

[[nodiscard]] std::string_view rockSpriteName(RockSize size)
{
    switch(size)
    {
    case RockSize::Large:
        return "rock/large";
    case RockSize::Medium:
        return "rock/medium";
    case RockSize::Small:
        return "rock/small";
    }
    return "rock/large";
}

[[nodiscard]] sgl::Vec2f topLeftFromCenter(sgl::Vec2f center, sgl::Vec2f size)
{
    return {center.x - size.x * 0.5f, center.y - size.y * 0.5f};
}

} // namespace

PlayScene::PlayScene(const AppServices& services)
    : services_{services}
    , game_{services.seed, services.tuning}
    , feedback_{services.audio, services.sounds}
{
}

void PlayScene::update(sgl::SceneContext& ctx, sgl::Seconds dt)
{
    if(game_.over())
    {
        return;
    }

    const sgl::InputState& input = ctx.input();
    if(input.action(services_.actions.pause).pressed)
    {
        ctx.request(sgl::RequestPause{});
        return;
    }

    const auto events = game_.step(toSimInput(input, services_.actions), dt);
    feedback_.onEvents(events);
    for(const AsteroidsEvent& event: events)
    {
        if(std::holds_alternative<Died>(event))
        {
            ctx.request(sgl::ReplaceScene{gameOver(services_, game_.score())});
            return;
        }
    }
}

void PlayScene::render(sgl::RenderQueue& queue) const
{
    const sgl::Vec2f bgSize{designWidth, designHeight};
    if(const auto bg = services_.sprites.sprite("bg/space", {0.f, 0.f}, bgSize, kWhite))
    {
        queue.push(sgl::Layer::Background, 0.f, *bg);
    }
    else
    {
        queue.push(sgl::Layer::Background,
                   0.f,
                   sgl::RectCmd{
                       .rect = {.pos = {0.f, 0.f}, .size = bgSize},
                       .fill = spaceBlack,
                   });
    }

    float depth = 0.f;
    for(const RockView& rock: game_.rocks())
    {
        const sgl::Vec2f size{rock.radius * 2.f, rock.radius * 2.f};
        const sgl::Vec2f center = toScreen(rock.pos);
        if(const auto sprite =
               services_.sprites.sprite(rockSpriteName(rock.size), topLeftFromCenter(center, size), size, kWhite))
        {
            queue.push(sgl::Layer::World, depth, *sprite);
        }
        else
        {
            queue.push(sgl::Layer::World,
                       depth,
                       sgl::CircleCmd{
                           .center = center,
                           .radius = rock.radius,
                           .fill = rockFill,
                       });
        }
        depth += 1.f;
    }

    for(const BulletView& bullet: game_.bullets())
    {
        constexpr float kBulletDraw = 8.f;
        const sgl::Vec2f size{kBulletDraw, kBulletDraw};
        const sgl::Vec2f center = toScreen(bullet.pos);
        if(const auto sprite = services_.sprites.sprite("bullet", topLeftFromCenter(center, size), size, kWhite))
        {
            queue.push(sgl::Layer::Actors, depth, *sprite);
        }
        else
        {
            queue.push(sgl::Layer::Actors,
                       depth,
                       sgl::CircleCmd{
                           .center = center,
                           .radius = bullet.radius,
                           .fill = bulletFill,
                       });
        }
        depth += 1.f;
    }

    {
        const ShipView ship = game_.ship();
        const float draw = ship.radius * 2.4f;
        const sgl::Vec2f size{draw, draw};
        const sgl::Vec2f center = toScreen(ship.pos);
        if(const auto sprite = services_.sprites.sprite(
               "ship/player", topLeftFromCenter(center, size), size, kWhite, shipRotationDeg(ship.heading)))
        {
            queue.push(sgl::Layer::Actors, depth, *sprite);
        }
        else
        {
            queue.push(sgl::Layer::Actors, depth, shipMesh(ship));
        }
    }

    queue.push(sgl::Layer::Hud,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {12.f, 8.f}, .size = {220.f, 40.f}},
                   .fill = scorePlate,
               });
    queue.push(sgl::Layer::Hud,
               1.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Score " + std::to_string(game_.score()),
                   .size = 24,
                   .position = scorePos,
                   .color = hudText,
               });
}

sgl::SceneTraits PlayScene::traits() const
{
    return sgl::SceneTraits{
        .opaque = true,
        .blocksUpdate = true,
        .pausable = true,
    };
}

AsteroidsGame& PlayScene::game()
{
    return game_;
}

const AsteroidsGame& PlayScene::game() const
{
    return game_;
}

} // namespace sgl::asteroids
