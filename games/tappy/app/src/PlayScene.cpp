#include <algorithm>
#include <cmath>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <string>
#include <tappy/app/InputMapping.hpp>
#include <tappy/app/PlayScene.hpp>
#include <tappy/app/Theme.hpp>
#include <tappy/sim/Events.hpp>
#include <tappy/sim/Types.hpp>
#include <variant>

namespace sgl::tappy
{
namespace
{

constexpr sgl::Color kWhite{255, 255, 255, 255};
constexpr float kHillsParallax{0.35f};

[[nodiscard]] float screenY(float worldY)
{
    return kWorldH - worldY;
}

[[nodiscard]] sgl::Rect<float> toScreen(Vec2f pos, Vec2f size, float cameraLeft)
{
    return sgl::Rect<float>{
        .pos = {pos.x - cameraLeft, screenY(pos.y + size.y)},
        .size = size,
    };
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
    for(const TappyEvent& event: events)
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
    const float camera = game_.cameraLeft();
    const sgl::Vec2f bgSize{designWidth, designHeight};

    if(const auto bg = services_.sprites.sprite("bg/sky", {0.f, 0.f}, bgSize, kWhite))
    {
        queue.push(sgl::Layer::Background, 0.f, *bg);
    }
    else
    {
        queue.push(sgl::Layer::Background,
                   0.f,
                   sgl::RectCmd{
                       .rect = {.pos = {0.f, 0.f}, .size = bgSize},
                       .fill = fieldFill,
                   });
    }

    const float hillsOffset = -std::fmod(camera * kHillsParallax, designWidth);
    if(const auto hills = services_.sprites.sprite("bg/hills", {hillsOffset, 420.f}, {designWidth, 300.f}, kWhite))
    {
        queue.push(sgl::Layer::Background, 1.f, *hills);
        auto hills2 = *hills;
        hills2.position.x += designWidth;
        queue.push(sgl::Layer::Background, 1.1f, hills2);
    }
    else
    {
        queue.push(sgl::Layer::Background,
                   1.f,
                   sgl::RectCmd{
                       .rect = {.pos = {hillsOffset, 480.f}, .size = {designWidth * 2.f, 240.f}},
                       .fill = hillsFill,
                   });
    }

    float depth = 0.f;
    for(const GateView& gate: game_.gates())
    {
        const sgl::Rect<float> top = toScreen(gate.topPos, gate.topSize, camera);
        const sgl::Rect<float> bottom = toScreen(gate.bottomPos, gate.bottomSize, camera);
        if(const auto sprite = services_.sprites.sprite("gate/rockTop", top.pos, top.size, kWhite))
        {
            queue.push(sgl::Layer::World, depth, *sprite);
        }
        else
        {
            queue.push(sgl::Layer::World, depth, sgl::RectCmd{.rect = top, .fill = gateFill});
        }
        depth += 1.f;
        if(const auto sprite = services_.sprites.sprite("gate/rockBottom", bottom.pos, bottom.size, kWhite))
        {
            queue.push(sgl::Layer::World, depth, *sprite);
        }
        else
        {
            queue.push(sgl::Layer::World, depth, sgl::RectCmd{.rect = bottom, .fill = gateFill});
        }
        depth += 1.f;
    }

    const PlayerView player = game_.player();
    const sgl::Rect<float> playerRect = toScreen(player.pos, player.size, camera);
    const float tilt = std::clamp(player.vel.y * 0.04f, -25.f, 35.f);
    if(auto sprite = services_.sprites.sprite("player/blue", playerRect.pos, playerRect.size, kWhite, tilt))
    {
        queue.push(sgl::Layer::Actors, 0.f, *sprite);
    }
    else
    {
        queue.push(sgl::Layer::Actors, 0.f, sgl::RectCmd{.rect = playerRect, .fill = playerFill});
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
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = true};
}

TappyGame& PlayScene::game()
{
    return game_;
}

const TappyGame& PlayScene::game() const
{
    return game_;
}

} // namespace sgl::tappy
