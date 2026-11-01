#include <dodger/app/InputMapping.hpp>
#include <dodger/app/PlayScene.hpp>
#include <dodger/app/Theme.hpp>
#include <dodger/sim/Events.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <string>
#include <variant>

namespace sgl::dodger
{

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
    for(const DodgerEvent& event: events)
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

    const PaddleView paddle = game_.paddle();
    queue.push(sgl::Layer::Actors,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = paddle.pos, .size = paddle.size},
                   .fill = paddleFill,
               });

    float depth = 1.f;
    for(const HazardView& hazard: game_.hazards())
    {
        queue.push(sgl::Layer::Actors,
                   depth,
                   sgl::RectCmd{
                       .rect = {.pos = hazard.pos, .size = hazard.size},
                       .fill = hazardFill,
                   });
        depth += 1.f;
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

DodgerGame& PlayScene::game()
{
    return game_;
}

const DodgerGame& PlayScene::game() const
{
    return game_;
}

} // namespace sgl::dodger
