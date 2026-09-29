#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <tetris/app/MainMenuScene.hpp>
#include <tetris/app/Theme.hpp>
#include <vector>

namespace sgl::tetris
{
namespace
{

constexpr sgl::Vec2f kButtonSize{400.f, 72.f};
constexpr float kStartY{340.f};
constexpr float kQuitY{440.f};
constexpr float kTitleY{180.f};
constexpr float kBestY{250.f};

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
        sgl::Button{.rect = centeredButton(kStartY), .label = "Start"},
        sgl::Button{.rect = centeredButton(kQuitY), .label = "Quit"},
    };
}

[[nodiscard]] sgl::MenuStyle makeStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 28,
        .idle = startButton,
        .focused = startButtonHover,
        .text = startLabel,
        .panel = panelBox,
        .cornerRadius = 0.f,
        .bodyFont = font,
    };
}

} // namespace

MainMenuScene::MainMenuScene(const AppServices& services)
    : services_{services}
    , model_{makeButtons()}
    , style_{makeStyle(services.font)}
{
}

void MainMenuScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    const sgl::MenuActions actions = menuActions(services_.actions);
    if(const auto activated = model_.update(ctx.input(), actions))
    {
        if(*activated == 0)
        {
            ctx.request(sgl::ReplaceScene{play(services_)});
        }
        else
        {
            ctx.request(sgl::QuitApp{});
        }
        return;
    }

    if(model_.cancelled())
    {
        ctx.request(sgl::QuitApp{});
    }
}

void MainMenuScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Hud,
               0.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Tetris",
                   .size = 56,
                   .position = {designWidth * 0.5f, kTitleY},
                   .color = titleGold,
                   .anchor = sgl::Anchor::Center,
               });

    const auto entries = services_.highScores.entries();
    const std::string bestText = entries.empty() ? "Best --" : ("Best " + std::to_string(entries.front()));
    queue.push(sgl::Layer::Hud,
               1.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = bestText,
                   .size = 24,
                   .position = {designWidth * 0.5f, kBestY},
                   .color = hudText,
                   .anchor = sgl::Anchor::Center,
               });

    sgl::renderMenu(queue, model_, style_, sgl::Layer::Hud);
}

sgl::SceneTraits MainMenuScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::tetris
