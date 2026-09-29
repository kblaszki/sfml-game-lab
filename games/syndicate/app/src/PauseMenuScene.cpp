#include <syndicate/app/PauseMenuScene.hpp>

#include <memory>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <sgl/ui/SettingsScene.hpp>
#include <syndicate/app/HelpScene.hpp>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr float kOverlayWidth{1280.f};
constexpr float kOverlayHeight{720.f};
constexpr float kButtonWidth{480.f};
constexpr float kButtonHeight{56.f};
constexpr float kButtonX{(kOverlayWidth - kButtonWidth) * 0.5f};
constexpr float kFirstY{240.f};
constexpr float kGap{16.f};

[[nodiscard]] sgl::Rect<float> buttonRect(std::size_t index)
{
    return sgl::Rect<float>{
        .pos = {kButtonX, kFirstY + static_cast<float>(index) * (kButtonHeight + kGap)},
        .size = {kButtonWidth, kButtonHeight},
    };
}

[[nodiscard]] std::vector<sgl::Button> makePauseButtons()
{
    return {
        sgl::Button{.rect = buttonRect(0), .label = "Resume"},
        sgl::Button{.rect = buttonRect(1), .label = "Settings"},
        sgl::Button{.rect = buttonRect(2), .label = "Help"},
        sgl::Button{.rect = buttonRect(3), .label = "Quit"},
    };
}

[[nodiscard]] sgl::MenuStyle makePauseStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 28,
        .idle = sgl::Color{48, 56, 88, 230},
        .focused = sgl::Color{72, 96, 160, 255},
        .text = sgl::Color{255, 255, 255},
        .panel = sgl::Color{24, 28, 48, 230},
        .cornerRadius = 0.f,
        .bodyFont = font,
    };
}

} // namespace

PauseMenuScene::PauseMenuScene(const SyndicateServices& services)
    : services_{services}
    , model_{makePauseButtons()}
    , style_{makePauseStyle(services.font)}
{
}

void PauseMenuScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    if(const auto activated = model_.update(ctx.input(), services_.actions.menu))
    {
        switch(*activated)
        {
        case 0:
            ctx.request(sgl::PopScene{});
            break;
        case 1:
            ctx.request(sgl::PushScene{[this]() {
                return std::make_unique<sgl::SettingsScene>(
                    services_.settings,
                    services_.actions.menu,
                    services_.font,
                    [this](const sgl::Settings& settings) {
                        services_.settings = settings;
                        if(services_.onSettingsChanged)
                        {
                            services_.onSettingsChanged(settings);
                        }
                    });
            }});
            break;
        case 2:
            ctx.request(sgl::PushScene{[actions = services_.actions.menu, font = services_.font]() {
                return std::make_unique<HelpScene>(0, actions, font);
            }});
            break;
        case 3:
            ctx.request(sgl::QuitApp{});
            break;
        default:
            break;
        }
        return;
    }

    if(model_.cancelled())
    {
        ctx.request(sgl::PopScene{});
    }
}

void PauseMenuScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Overlay,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {kOverlayWidth, kOverlayHeight}},
                   .fill = sgl::Color{0, 0, 0, 150},
               });
    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits PauseMenuScene::traits() const
{
    return sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::syndicate
