#include <algorithm>
#include <cstdint>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/save/SaveError.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/RenderMenu.hpp>
#include <string>
#include <syndicate/app/GameOverScene.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr float kDesignWidth{1280.f};
constexpr float kDesignHeight{720.f};
constexpr sgl::Vec2f kButtonSize{360.f, 64.f};

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
        sgl::Button{.rect = centeredButton(520.f), .label = "Menu"},
        sgl::Button{.rect = centeredButton(600.f), .label = "Quit"},
    };
}

[[nodiscard]] sgl::MenuStyle makeStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 26,
        .idle = sgl::Color{48, 56, 88, 230},
        .focused = sgl::Color{72, 96, 160, 255},
        .text = sgl::Color{255, 255, 255},
        .panel = sgl::Color{24, 28, 48, 230},
        .cornerRadius = 0.f,
        .bodyFont = font,
    };
}

[[nodiscard]] std::string playerLabel(PlayerId id)
{
    return "P" + std::to_string(std::to_underlying(id) + 1);
}

} // namespace

GameOverScene::GameOverScene(GameOver over, sgl::FontId font)
    : over_{std::move(over)}
    , font_{font}
    , model_{makeButtons()}
    , style_{makeStyle(font)}
{
}

GameOverScene::GameOverScene(const SyndicateServices& services, GameOver over)
    : services_{services}
    , over_{std::move(over)}
    , font_{services.font}
    , model_{makeButtons()}
    , style_{makeStyle(services.font)}
{
    trySaveScore();
}

void GameOverScene::trySaveScore()
{
    if(!services_.has_value() || over_.ranking.empty())
    {
        return;
    }
    const RankingEntry& winner = over_.ranking.front();
    const auto raw = std::max<std::int64_t>(0, winner.net.v);
    const auto score = static_cast<std::uint32_t>(std::min<std::int64_t>(raw, 0xffffffffLL));
    if(services_->highScores.insert(score).has_value())
    {
        const auto written = sgl::ScoreStore::save(services_->scoresPath, services_->highScores);
        if(!written)
        {
            saveError_ = sgl::describe(written.error());
        }
    }
}

void GameOverScene::update(sgl::SceneContext& ctx, sgl::Seconds)
{
    const sgl::MenuActions menu = services_.has_value() ? services_->actions.menu
                                                       : sgl::MenuActions{};
    if(const auto activated = model_.update(ctx.input(), menu))
    {
        if(*activated == 0)
        {
            if(services_.has_value())
            {
                ctx.request(sgl::PopScene{});
                ctx.request(sgl::ReplaceScene{mainMenu(*services_)});
            }
            else
            {
                ctx.request(sgl::PopScene{});
            }
        }
        else
        {
            ctx.request(sgl::QuitApp{});
        }
        return;
    }

    if(model_.cancelled())
    {
        if(services_.has_value())
        {
            ctx.request(sgl::PopScene{});
            ctx.request(sgl::ReplaceScene{mainMenu(*services_)});
        }
        else
        {
            ctx.request(sgl::PopScene{});
        }
    }
}

void GameOverScene::render(sgl::RenderQueue& queue) const
{
    float depth = 0.f;
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {kDesignWidth, kDesignHeight}},
                   .fill = sgl::Color{0, 0, 0, 150},
               });
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font_,
                   .text = "Ranking",
                   .size = 36,
                   .position = {kDesignWidth * 0.5f, 100.f},
                   .color = sgl::Color{255, 230, 160},
                   .anchor = sgl::Anchor::Center,
               });

    float y = 170.f;
    for(std::size_t i = 0; i < over_.ranking.size(); ++i)
    {
        const RankingEntry& e = over_.ranking[i];
        const std::string line = "#" + std::to_string(i + 1) + " " + playerLabel(e.player) + " net " +
                                 std::to_string(e.net.v);
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = font_,
                       .text = line,
                       .size = 22,
                       .position = {kDesignWidth * 0.5f, y},
                       .color = sgl::Color{220, 220, 230},
                       .anchor = sgl::Anchor::Center,
                   });
        y += 36.f;
    }

    if(!saveError_.empty())
    {
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = font_,
                       .text = std::string{"Save failed: "} + saveError_,
                       .size = 18,
                       .position = {kDesignWidth * 0.5f, 460.f},
                       .color = sgl::Color{220, 100, 100},
                       .anchor = sgl::Anchor::Center,
                   });
    }

    sgl::renderMenu(queue, model_, style_, sgl::Layer::Overlay);
}

sgl::SceneTraits GameOverScene::traits() const
{
    return sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::syndicate
