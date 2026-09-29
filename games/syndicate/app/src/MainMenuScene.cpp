#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <syndicate/app/MainMenuScene.hpp>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr float kDesignWidth{1280.f};
constexpr sgl::Vec2f kButtonSize{400.f, 72.f};
constexpr float kPlayY{340.f};
constexpr float kQuitY{440.f};
constexpr float kTitleY{180.f};
constexpr float kBestY{250.f};

[[nodiscard]] sgl::Rect<float> centeredButton(float y)
{
    return sgl::Rect<float>{
        .pos = {(kDesignWidth - kButtonSize.x) * 0.5f, y},
        .size = kButtonSize,
    };
}

[[nodiscard]] std::vector<sgl::Button> makeButtons()
{
    return {
        sgl::Button{.rect = centeredButton(kPlayY), .label = "Play"},
        sgl::Button{.rect = centeredButton(kQuitY), .label = "Quit"},
    };
}

[[nodiscard]] sgl::MenuStyle makeStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 28,
        .idle = sgl::Color{48, 56, 88, 230},
        .focused = sgl::Color{72, 96, 160, 255},
        .text = sgl::Color{255, 255, 255},
        .panel = sgl::Color{24, 28, 48, 230},
    };
}

} // namespace

MainMenuScene::MainMenuScene(const SyndicateServices& services)
    : services_{services}
    , model_{makeButtons()}
    , style_{makeStyle(services.font)}
{
}

void MainMenuScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    if(const auto activated = model_.update(ctx.input(), services_.actions.menu))
    {
        if(*activated == 0)
        {
            ctx.request(sgl::PushScene{setup(services_)});
        }
        else
        {
            ctx.request(sgl::QuitApp{});
        }
        return;
    }

    if(model_.cancelled() || ctx.input().action(services_.actions.pause).pressed)
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
                   .text = "Syndicate",
                   .size = 56,
                   .position = {kDesignWidth * 0.5f, kTitleY},
                   .color = sgl::Color{255, 230, 160},
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
                   .position = {kDesignWidth * 0.5f, kBestY},
                   .color = sgl::Color{240, 230, 210},
                   .anchor = sgl::Anchor::Center,
               });

    sgl::renderMenu(queue, model_, style_, sgl::Layer::Hud);
}

sgl::SceneTraits MainMenuScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::syndicate
