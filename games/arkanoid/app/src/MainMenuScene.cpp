#include <arkanoid/app/MainMenuScene.hpp>
#include <arkanoid/app/Theme.hpp>
#include <arkanoid/sim/Tuning.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <vector>

namespace sgl::arkanoid
{
namespace
{

constexpr sgl::Vec2f kButtonSize{400.f, 72.f};
constexpr float kStartY{320.f};
constexpr float kQuitY{420.f};
constexpr float kTitleY{200.f};
constexpr float kBestY{260.f};

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
        .panel = sgl::Color{24, 28, 48, 230},
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
            ctx.request(sgl::ReplaceScene{gameplay(services_, StageId::Stage1)});
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
                   .text = "Breakout",
                   .size = 56,
                   .position = {designWidth * 0.5f, kTitleY},
                   .color = titleGold,
                   .anchor = sgl::Anchor::Center,
               });

    const auto entries = services_.highScores.entries();
    const std::uint32_t best = entries.empty() ? 0u : entries.front();
    queue.push(sgl::Layer::Hud,
               1.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Best " + std::to_string(best),
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

} // namespace sgl::arkanoid
