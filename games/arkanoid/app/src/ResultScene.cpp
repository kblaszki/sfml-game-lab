#include <arkanoid/app/ResultScene.hpp>
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
        .panel = sgl::Color{24, 28, 48, 230},
        .cornerRadius = 0.f,
        .bodyFont = font,
    };
}

} // namespace

ResultScene::ResultScene(
    const AppServices& services, Outcome outcome, std::uint32_t score, std::optional<std::size_t> rank, bool saveFailed)
    : services_{services}
    , outcome_{outcome}
    , score_{score}
    , rank_{rank}
    , saveFailed_{saveFailed}
    , model_{makeButtons()}
    , style_{makeStyle(services.font)}
{
}

Outcome ResultScene::outcome() const noexcept
{
    return outcome_;
}

std::uint32_t ResultScene::score() const noexcept
{
    return score_;
}

std::optional<std::size_t> ResultScene::rank() const noexcept
{
    return rank_;
}

bool ResultScene::saveFailed() const noexcept
{
    return saveFailed_;
}

void ResultScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    const sgl::MenuActions actions = menuActions(services_.actions);
    if(const auto activated = model_.update(ctx.input(), actions))
    {
        if(*activated == 0)
        {
            ctx.request(sgl::PopScene{});
            ctx.request(sgl::ReplaceScene{gameplay(services_, StageId::Stage1)});
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

void ResultScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Overlay,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {designWidth, designHeight}},
                   .fill = sgl::Color{0, 0, 0, 150},
               });

    const char* title = outcome_ == Outcome::Won ? "You win" : "You lose";
    queue.push(sgl::Layer::Overlay,
               1.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = title,
                   .size = 42,
                   .position = {designWidth * 0.5f, 200.f},
                   .color = titleGold,
                   .anchor = sgl::Anchor::Center,
               });
    queue.push(sgl::Layer::Overlay,
               2.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Score " + std::to_string(score_),
                   .size = 28,
                   .position = {designWidth * 0.5f, 260.f},
                   .color = hudText,
                   .anchor = sgl::Anchor::Center,
               });

    float nextY = 310.f;
    if(rank_.has_value())
    {
        queue.push(sgl::Layer::Overlay,
                   3.f,
                   sgl::TextCmd{
                       .font = services_.font,
                       .text = "New record",
                       .size = 24,
                       .position = {designWidth * 0.5f, nextY},
                       .color = bannerTitle,
                       .anchor = sgl::Anchor::Center,
                   });
        nextY += 40.f;
    }
    if(saveFailed_)
    {
        queue.push(sgl::Layer::Overlay,
                   4.f,
                   sgl::TextCmd{
                       .font = services_.font,
                       .text = "Could not save high score",
                       .size = 20,
                       .position = {designWidth * 0.5f, nextY},
                       .color = bannerHint,
                       .anchor = sgl::Anchor::Center,
                   });
    }

    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits ResultScene::traits() const
{
    return sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::arkanoid
