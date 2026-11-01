#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <snake/app/PauseScene.hpp>
#include <snake/app/Theme.hpp>
#include <vector>

namespace sgl::snake
{
namespace
{

constexpr sgl::Rect<float> kResumeRect{.pos = {400.f, 320.f}, .size = {480.f, 64.f}};
constexpr sgl::Rect<float> kMenuRect{.pos = {400.f, 410.f}, .size = {480.f, 64.f}};

[[nodiscard]] std::vector<sgl::Button> makeButtons()
{
    return {
        sgl::Button{.rect = kResumeRect, .label = "Resume"},
        sgl::Button{.rect = kMenuRect, .label = "Menu"},
    };
}

[[nodiscard]] sgl::MenuStyle makeStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 28,
        .idle = menuButton,
        .focused = menuButtonFocus,
        .text = menuLabel,
        .panel = panelBox,
        .cornerRadius = 0.f,
        .bodyFont = font,
    };
}

} // namespace

PauseScene::PauseScene(const AppServices& services)
    : services_{services}
    , model_{makeButtons()}
    , style_{makeStyle(services.font)}
{
}

void PauseScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    const sgl::InputState& input = ctx.input();
    if(input.action(services_.actions.pause).pressed)
    {
        ctx.request(sgl::PopScene{});
        return;
    }

    const sgl::MenuActions actions = menuActions(services_.actions);
    if(const auto activated = model_.update(input, actions))
    {
        if(*activated == 0)
        {
            ctx.request(sgl::PopScene{});
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
    }
}

void PauseScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Overlay,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {designWidth, designHeight}},
                   .fill = overlayDim,
               });
    queue.push(sgl::Layer::Overlay,
               1.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Paused",
                   .size = 42,
                   .position = {designWidth * 0.5f, 260.f},
                   .color = titleGreen,
                   .anchor = sgl::Anchor::Center,
               });
    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits PauseScene::traits() const
{
    return sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::snake
