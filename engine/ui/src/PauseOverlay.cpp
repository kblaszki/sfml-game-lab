#include <sgl/ui/PauseOverlay.hpp>

#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <vector>

namespace sgl
{
namespace
{

constexpr float kOverlayWidth{1280.f};
constexpr float kOverlayHeight{720.f};
constexpr Rect<float> kResumeRect{.pos = {400.f, 320.f}, .size = {480.f, 64.f}};
constexpr Rect<float> kQuitRect{.pos = {400.f, 410.f}, .size = {480.f, 64.f}};

[[nodiscard]] std::vector<Button> makePauseButtons()
{
    return {
        Button{.rect = kResumeRect, .label = "Resume"},
        Button{.rect = kQuitRect, .label = "Quit"},
    };
}

[[nodiscard]] MenuStyle makePauseStyle(FontId font)
{
    return MenuStyle{
        .font = font,
        .textSize = 28,
        .idle = Color{48, 56, 88, 230},
        .focused = Color{72, 96, 160, 255},
        .text = Color{255, 255, 255},
        .panel = Color{24, 28, 48, 230},
    };
}

} // namespace

PauseOverlay::PauseOverlay(MenuActions actions, FontId font)
    : actions_{actions}
    , model_{makePauseButtons()}
    , style_{makePauseStyle(font)}
{
}

void PauseOverlay::update(SceneContext& ctx, Seconds)
{
    if(const auto activated = model_.update(ctx.input(), actions_))
    {
        if(*activated == 0)
        {
            ctx.request(PopScene{});
        }
        else
        {
            ctx.request(QuitApp{});
        }
        return;
    }

    if(model_.cancelled())
    {
        ctx.request(PopScene{});
    }
}

void PauseOverlay::render(RenderQueue& queue) const
{
    queue.push(Layer::Overlay,
               0.f,
               RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {kOverlayWidth, kOverlayHeight}},
                   .fill = Color{0, 0, 0, 150},
               });
    renderMenu(queue, model_, style_, Layer::Overlay);
}

SceneTraits PauseOverlay::traits() const
{
    return SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl
