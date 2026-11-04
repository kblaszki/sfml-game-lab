#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <tower/app/GameOverScene.hpp>
#include <tower/app/Theme.hpp>
#include <vector>

namespace sgl::tower
{
namespace
{

constexpr sgl::Vec2f kButtonSize{360.f, 64.f};
constexpr float kMenuY{460.f};

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
}

int GameOverScene::score() const noexcept
{
    return score_;
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
                   .rect = {.pos = {340.f, 180.f}, .size = {600.f, 360.f}},
                   .fill = panelBox,
               });
    queue.push(sgl::Layer::Overlay,
               2.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Game Over",
                   .size = 42,
                   .position = {designWidth * 0.5f, 240.f},
                   .color = titleIce,
                   .anchor = sgl::Anchor::Center,
               });
    queue.push(sgl::Layer::Overlay,
               3.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Score " + std::to_string(score_),
                   .size = 28,
                   .position = {designWidth * 0.5f, 320.f},
                   .color = hudText,
                   .anchor = sgl::Anchor::Center,
               });
    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits GameOverScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::tower
