#include <arkanoid/app/PauseScene.hpp>
#include <arkanoid/sim/Tuning.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <vector>

namespace sgl::arkanoid
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
        .idle = sgl::Color{48, 56, 88, 230},
        .focused = sgl::Color{72, 96, 160, 255},
        .text = sgl::Color{255, 255, 255},
        .panel = sgl::Color{24, 28, 48, 230},
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
    const Actions& actions = services_.actions;

    // Pointer (and up/down focus) via MenuModel; confirm stays Enter→menu like the prior overlay.
    sgl::MenuActions menu = menuActions(actions);
    menu.confirm = sgl::ActionId{};

    if(const auto activated = model_.update(input, menu))
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

    if(model_.cancelled() || input.action(actions.pause).pressed)
    {
        ctx.request(sgl::PopScene{});
        return;
    }

    if(input.action(actions.confirm).pressed)
    {
        ctx.request(sgl::PopScene{});
        ctx.request(sgl::ReplaceScene{mainMenu(services_)});
    }
}

void PauseScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Overlay,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {designWidth, designHeight}},
                   .fill = sgl::Color{0, 0, 0, 150},
               });
    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits PauseScene::traits() const
{
    return sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::arkanoid
