#include <cstddef>
#include <cstdint>
#include <dodger/app/GameOverScene.hpp>
#include <dodger/app/Theme.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <vector>

namespace sgl::dodger
{
namespace
{

constexpr sgl::Vec2f kButtonSize{360.f, 64.f};
constexpr float kMenuY{520.f};
constexpr float kRankStartY{290.f};
constexpr float kRankStep{28.f};
constexpr sgl::Color kSaveError{220, 90, 80};

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
        sgl::Button{.rect = centeredButton(kMenuY), .label = "Menu"},
    };
}

[[nodiscard]] sgl::MenuStyle makeStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 26,
        .idle = menuButton,
        .focused = menuButtonFocus,
        .text = menuLabel,
        .panel = panelBox,
        .cornerRadius = 0.f,
        .bodyFont = font,
    };
}

} // namespace

GameOverScene::GameOverScene(const AppServices& services, int score)
    : services_{services}
    , score_{score}
    , model_{makeButtons()}
    , style_{makeStyle(services.font)}
{
    rank_ = services_.highScores.insert(static_cast<std::uint32_t>(score_));
    if(rank_.has_value())
    {
        const auto written = sgl::ScoreStore::save(services_.scoresPath, services_.highScores);
        if(!written)
        {
            saveError_ = sgl::describe(written.error());
        }
    }
}

int GameOverScene::score() const noexcept
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
        ctx.request(sgl::ReplaceScene{mainMenu(services_)});
        return;
    }

    if(model_.cancelled())
    {
        ctx.request(sgl::ReplaceScene{mainMenu(services_)});
    }
}

void GameOverScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Background,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {designWidth, designHeight}},
                   .fill = fieldFill,
               });
    queue.push(sgl::Layer::Overlay,
               1.f,
               sgl::RectCmd{
                   .rect = {.pos = {340.f, 100.f}, .size = {600.f, 500.f}},
                   .fill = panelBox,
               });
    queue.push(sgl::Layer::Overlay,
               2.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Game Over",
                   .size = 42,
                   .position = {designWidth * 0.5f, 160.f},
                   .color = titleAmber,
                   .anchor = sgl::Anchor::Center,
               });
    queue.push(sgl::Layer::Overlay,
               3.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Score " + std::to_string(score_),
                   .size = 28,
                   .position = {designWidth * 0.5f, 230.f},
                   .color = hudText,
                   .anchor = sgl::Anchor::Center,
               });

    const auto entries = services_.highScores.entries();
    float depth = 4.f;
    float y = kRankStartY;
    for(std::size_t i = 0; i < entries.size(); ++i)
    {
        const bool highlighted = rank_.has_value() && *rank_ == i;
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = services_.font,
                       .text = "#" + std::to_string(i + 1) + " " + std::to_string(entries[i]),
                       .size = 22,
                       .position = {designWidth * 0.5f, y},
                       .color = highlighted ? titleAmber : hudText,
                       .anchor = sgl::Anchor::Center,
                   });
        y += kRankStep;
    }

    if(rank_.has_value())
    {
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = services_.font,
                       .text = "New record",
                       .size = 22,
                       .position = {designWidth * 0.5f, y},
                       .color = titleAmber,
                       .anchor = sgl::Anchor::Center,
                   });
        y += kRankStep;
    }

    if(!saveError_.empty())
    {
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = services_.font,
                       .text = std::string{"Save failed: "} + saveError_,
                       .size = 18,
                       .position = {designWidth * 0.5f, y},
                       .color = kSaveError,
                       .anchor = sgl::Anchor::Center,
                   });
    }

    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits GameOverScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::dodger
