#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <syndicate/app/SetupScene.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr float kDesignWidth{1280.f};
constexpr sgl::Vec2f kButtonSize{420.f, 56.f};
constexpr float kStartY{180.f};
constexpr float kRowStep{70.f};

[[nodiscard]] sgl::Rect<float> centeredButton(float y)
{
    return sgl::Rect<float>{
        .pos = {(kDesignWidth - kButtonSize.x) * 0.5f, y},
        .size = kButtonSize,
    };
}

[[nodiscard]] sgl::MenuStyle makeStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 24,
        .idle = sgl::Color{48, 56, 88, 230},
        .focused = sgl::Color{72, 96, 160, 255},
        .text = sgl::Color{255, 255, 255},
        .panel = sgl::Color{24, 28, 48, 230},
        .cornerRadius = 0.f,
        .bodyFont = font,
    };
}

} // namespace

SetupScene::SetupScene(const SyndicateServices& services)
    : services_{services}
    , model_{std::vector<sgl::Button>{}}
    , style_{makeStyle(services.font)}
{
    rebuildModel();
}

void SetupScene::rebuildModel()
{
    std::vector<sgl::Button> buttons;
    buttons.reserve(static_cast<std::size_t>(seatCount_) + 2u);
    buttons.push_back(sgl::Button{
        .rect = centeredButton(kStartY),
        .label = "Seats: " + std::to_string(seatCount_),
    });
    for(int i = 0; i < seatCount_; ++i)
    {
        const float y = kStartY + kRowStep * static_cast<float>(i + 1);
        buttons.push_back(sgl::Button{
            .rect = centeredButton(y),
            .label = "P" + std::to_string(i + 1) + ": " + (bots_[static_cast<std::size_t>(i)] ? "Bot" : "Human"),
        });
    }
    buttons.push_back(sgl::Button{
        .rect = centeredButton(kStartY + kRowStep * static_cast<float>(seatCount_ + 1)),
        .label = "Confirm",
    });
    model_ = sgl::MenuModel{std::move(buttons)};
}

MatchConfig SetupScene::makeConfig() const
{
    return MatchConfig{
        .seed = services_.seed,
        .seats = seatCount_,
        .startCash = Money{1500},
        .maxRounds = 15,
        .targetNetWorth = Money{6000},
    };
}

std::vector<Seat> SetupScene::makeSeats() const
{
    std::vector<Seat> seats;
    seats.reserve(static_cast<std::size_t>(seatCount_));
    for(int i = 0; i < seatCount_; ++i)
    {
        seats.push_back(Seat{.bot = bots_[static_cast<std::size_t>(i)]});
    }
    return seats;
}

void SetupScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    if(const auto activated = model_.update(ctx.input(), services_.actions.menu))
    {
        if(*activated == 0)
        {
            seatCount_ = seatCount_ == 4 ? 2 : seatCount_ + 1;
            rebuildModel();
            return;
        }
        const std::size_t confirmIndex = static_cast<std::size_t>(seatCount_) + 1u;
        if(*activated == confirmIndex)
        {
            ctx.request(sgl::ReplaceScene{play(services_, makeConfig(), makeSeats())});
            return;
        }
        const std::size_t seatIndex = *activated - 1u;
        if(std::cmp_less(seatIndex, seatCount_))
        {
            bots_[seatIndex] = !bots_[seatIndex];
            rebuildModel();
        }
        return;
    }

    if(model_.cancelled() || ctx.input().action(services_.actions.pause).pressed)
    {
        ctx.request(sgl::PopScene{});
    }
}

void SetupScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Hud,
               0.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Setup",
                   .size = 42,
                   .position = {kDesignWidth * 0.5f, 100.f},
                   .color = sgl::Color{255, 230, 160},
                   .anchor = sgl::Anchor::Center,
               });
    sgl::renderMenu(queue, model_, style_, sgl::Layer::Hud);
}

sgl::SceneTraits SetupScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::syndicate
