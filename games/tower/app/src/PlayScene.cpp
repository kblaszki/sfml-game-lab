#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <string>
#include <tower/app/InputMapping.hpp>
#include <tower/app/PlayScene.hpp>
#include <tower/app/Theme.hpp>
#include <tower/sim/Events.hpp>
#include <tower/sim/Types.hpp>
#include <variant>

namespace sgl::tower
{
namespace
{

// Sim +Y is up and pos is the bottom-left. RectCmd pos is the screen top-left, +Y down.
[[nodiscard]] float screenY(float worldY, float cameraBottom)
{
    return kViewH - (worldY - cameraBottom);
}

[[nodiscard]] sgl::Rect<float> toScreen(Vec2f pos, Vec2f size, float cameraBottom)
{
    return sgl::Rect<float>{
        .pos = {pos.x, screenY(pos.y + size.y, cameraBottom)},
        .size = size,
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
    for(const TowerEvent& event: events)
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
                   .fill = fieldFill,
               });

    const float camera = game_.cameraBottom();
    float depth = 0.f;
    for(const PlatformView& platform: game_.platforms())
    {
        queue.push(sgl::Layer::World,
                   depth,
                   sgl::RectCmd{
                       .rect = toScreen(platform.pos, platform.size, camera),
                       .fill = platformFill,
                   });
        depth += 1.f;
    }

    const PlayerView player = game_.player();
    queue.push(sgl::Layer::Actors,
               0.f,
               sgl::RectCmd{
                   .rect = toScreen(player.pos, player.size, camera),
                   .fill = playerFill,
               });

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

TowerGame& PlayScene::game()
{
    return game_;
}

const TowerGame& PlayScene::game() const
{
    return game_;
}

} // namespace sgl::tower
