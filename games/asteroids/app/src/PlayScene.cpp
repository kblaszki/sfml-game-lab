#include <asteroids/app/InputMapping.hpp>
#include <asteroids/app/PlayScene.hpp>
#include <asteroids/app/Theme.hpp>
#include <asteroids/sim/Events.hpp>
#include <cmath>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <string>
#include <variant>

namespace sgl::asteroids
{
namespace
{

[[nodiscard]] sgl::Vec2f toScreen(Vec2f sim)
{
    return {sim.x, kWorldH - sim.y};
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

} // namespace

PlayScene::PlayScene(const AppServices& services)
    : services_{services}
    , game_{services.seed, services.tuning}
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
    queue.push(sgl::Layer::Background,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {designWidth, designHeight}},
                   .fill = spaceBlack,
               });

    float depth = 0.f;
    for(const RockView& rock: game_.rocks())
    {
        queue.push(sgl::Layer::World,
                   depth,
                   sgl::CircleCmd{
                       .center = toScreen(rock.pos),
                       .radius = rock.radius,
                       .fill = rockFill,
                   });
        depth += 1.f;
    }

    for(const BulletView& bullet: game_.bullets())
    {
        queue.push(sgl::Layer::Actors,
                   depth,
                   sgl::CircleCmd{
                       .center = toScreen(bullet.pos),
                       .radius = bullet.radius,
                       .fill = bulletFill,
                   });
        depth += 1.f;
    }

    queue.push(sgl::Layer::Actors, depth, shipMesh(game_.ship()));

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
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = true};
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
