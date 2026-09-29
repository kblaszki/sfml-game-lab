#include <arkanoid/app/GameplayScene.hpp>
#include <arkanoid/app/SceneRender.hpp>
#include <arkanoid/app/Theme.hpp>
#include <arkanoid/sim/Levels.hpp>
#include <arkanoid/sim/Physics.hpp>
#include <arkanoid/sim/Tuning.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <variant>
#include <vector>

namespace sgl::arkanoid
{
namespace
{

constexpr std::uint64_t kFeedbackSeed{42};

} // namespace

GameplayScene::GameplayScene(const AppServices& services, StageId stage)
    : services_{services}
    , state_{makeState(stage)}
    , feedback_{services.audio, services.sounds, sgl::Pcg32{kFeedbackSeed}}
{
}

void GameplayScene::advanceFrom(StageId cleared)
{
    const std::uint32_t score = state_.score;
    const std::uint32_t lives = state_.lives;
    if(cleared == StageId::Stage1)
    {
        state_ = makeState(StageId::Stage2, score, lives);
    }
    else if(cleared == StageId::Stage2)
    {
        state_ = makeState(StageId::Stage3, score, lives);
    }
}

void GameplayScene::finishRound(sgl::SceneContext& ctx, Outcome outcome)
{
    const std::optional<std::size_t> rank = services_.highScores.insert(state_.score);
    const bool saveFailed = !sgl::ScoreStore::save(services_.scoresPath, services_.highScores).has_value();
    ctx.request(sgl::PushScene{result(services_, outcome, state_.score, rank, saveFailed)});
}

void GameplayScene::update(sgl::SceneContext& ctx, sgl::Seconds dt)
{
    const sgl::InputState& input = ctx.input();
    const Actions& actions = services_.actions;

    const SimInput simInput{
        .paddleAxis = input.axis(actions.paddle),
        .launch = input.action(actions.confirm).pressed,
    };
    const std::vector<SimEvent> events = step(state_, simInput, dt);
    feedback_.onEvents(events, state_);
    feedback_.update(dt);

    for(const SimEvent& event: events)
    {
        if(const auto* cleared = std::get_if<StageCleared>(&event))
        {
            if(cleared->stage == StageId::Stage1 || cleared->stage == StageId::Stage2)
            {
                advanceFrom(cleared->stage);
            }
            else if(cleared->stage == StageId::Stage3)
            {
                finishRound(ctx, Outcome::Won);
                return;
            }
        }
        if(std::holds_alternative<GameOver>(event))
        {
            finishRound(ctx, Outcome::Lost);
            return;
        }
    }

    if(!state_.cleared && !state_.over && input.action(actions.pause).pressed)
    {
        ctx.request(sgl::RequestPause{});
    }
}

void GameplayScene::render(sgl::RenderQueue& queue) const
{
    renderState(state_, services_.textures, queue);

    const HudModel model = hud();

    queue.push(sgl::Layer::Hud,
               0.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = model.score,
                   .size = 22,
                   .position = hudScorePos,
                   .color = hudText,
               });
    queue.push(sgl::Layer::Hud,
               1.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = model.lives,
                   .size = 22,
                   .position = hudLivesPos,
                   .color = hudText,
               });
    queue.push(sgl::Layer::Hud,
               2.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = model.effect,
                   .size = 22,
                   .position = hudPowerPos,
                   .color = hudText,
               });

    feedback_.render(queue);
}

sgl::SceneTraits GameplayScene::traits() const
{
    return sgl::SceneTraits{
        .opaque = true,
        .blocksUpdate = true,
        .pausable = !state_.cleared && !state_.over,
    };
}

State& GameplayScene::state()
{
    return state_;
}

const State& GameplayScene::state() const
{
    return state_;
}

HudModel GameplayScene::hud() const
{
    return makeHud(state_);
}

} // namespace sgl::arkanoid
