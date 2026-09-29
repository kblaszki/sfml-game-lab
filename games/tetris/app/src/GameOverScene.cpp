#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <tetris/app/GameOverScene.hpp>
#include <tetris/app/Theme.hpp>
#include <vector>

namespace sgl::tetris
{
namespace
{

constexpr sgl::Vec2f kButtonSize{360.f, 64.f};
constexpr float kRetryY{400.f};
constexpr float kMenuY{480.f};

[[nodiscard]] sgl::Rect<float> centeredButton(float y)
{
    return sgl::Rect<float>{
        .pos = {(designWidth - kButtonSize.x) * 0.5f, y},
        .size = kButtonSize,
    };
}

[[nodiscard]] std::vector<sgl::Button> makeButtons()
{
    return {
        sgl::Button{.rect = centeredButton(kRetryY), .label = "Retry"},
        sgl::Button{.rect = centeredButton(kMenuY), .label = "Menu"},
    };
}

[[nodiscard]] sgl::MenuStyle makeStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 26,
        .idle = startButton,
        .focused = startButtonHover,
        .text = startLabel,
        .panel = panelBox,
    };
}

} // namespace

GameOverScene::GameOverScene(const AppServices& services, std::uint32_t score)
    : services_{services}
    , score_{score}
    , model_{makeButtons()}
    , style_{makeStyle(services.font)}
{
    rank_ = services_.highScores.insert(score_);
    if(rank_.has_value())
    {
        const auto written = sgl::ScoreStore::save(services_.scoresPath, services_.highScores);
        if(!written)
        {
            saveError_ = sgl::describe(written.error());
        }
    }
}

std::uint32_t GameOverScene::score() const noexcept
{
    return score_;
}

bool GameOverScene::isNewRecord() const noexcept
{
    return rank_.has_value();
}

bool GameOverScene::saveFailed() const noexcept
{
    return !saveError_.empty();
}

void GameOverScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    const sgl::MenuActions actions = menuActions(services_.actions);
    if(const auto activated = model_.update(ctx.input(), actions))
    {
        if(*activated == 0)
        {
            ctx.request(sgl::PopScene{});
            ctx.request(sgl::ReplaceScene{play(services_)});
        }
        else
        {
            ctx.request(sgl::PopScene{});
            ctx.request(sgl::ReplaceScene{mainMenu(services_)});
        }
        return;
    }

    if(model_.cancelled())
    {
        ctx.request(sgl::PopScene{});
        ctx.request(sgl::ReplaceScene{mainMenu(services_)});
    }
}

void GameOverScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Overlay,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {designWidth, designHeight}},
                   .fill = overlayDim,
               });
    queue.push(sgl::Layer::Overlay,
               1.f,
               sgl::RectCmd{
                   .rect = {.pos = {340.f, 120.f}, .size = {600.f, 260.f}},
                   .fill = panelBox,
               });
    queue.push(sgl::Layer::Overlay,
               2.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Game Over",
                   .size = 42,
                   .position = {designWidth * 0.5f, 180.f},
                   .color = titleGold,
                   .anchor = sgl::Anchor::Center,
               });
    queue.push(sgl::Layer::Overlay,
               3.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Score " + std::to_string(score_),
                   .size = 28,
                   .position = {designWidth * 0.5f, 240.f},
                   .color = hudText,
                   .anchor = sgl::Anchor::Center,
               });

    const auto entries = services_.highScores.entries();
    const std::string bestText = entries.empty() ? "Best --" : ("Best " + std::to_string(entries.front()));
    queue.push(sgl::Layer::Overlay,
               4.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = bestText,
                   .size = 24,
                   .position = {designWidth * 0.5f, 280.f},
                   .color = hudText,
                   .anchor = sgl::Anchor::Center,
               });

    float depth = 5.f;
    if(rank_.has_value())
    {
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = services_.font,
                       .text = "New record",
                       .size = 24,
                       .position = {designWidth * 0.5f, 320.f},
                       .color = titleGold,
                       .anchor = sgl::Anchor::Center,
                   });
    }

    if(!saveError_.empty())
    {
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = services_.font,
                       .text = std::string{"Save failed: "} + saveError_,
                       .size = 18,
                       .position = {designWidth * 0.5f, 350.f},
                       .color = quitButtonHover,
                       .anchor = sgl::Anchor::Center,
                   });
    }

    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits GameOverScene::traits() const
{
    return sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::tetris
