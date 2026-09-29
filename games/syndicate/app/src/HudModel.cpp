#include <algorithm>
#include <cmath>
#include <cstddef>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <string>
#include <syndicate/app/HudModel.hpp>
#include <utility>
#include <variant>

namespace sgl::syndicate
{

void HudModel::pushNote(std::string text, float ttl)
{
    notes_.push_back(HudNote{.text = std::move(text), .ttl = ttl});
}

void HudModel::update(sgl::Seconds dt)
{
    const float elapsed = dt.count();
    for(HudNote& note: notes_)
    {
        note.ttl -= elapsed;
    }
    std::erase_if(notes_, [](const HudNote& n) { return n.ttl <= 0.f; });
}

std::string HudModel::phasePrompt(const Phase& phase) const
{
    if(std::holds_alternative<AwaitRoll>(phase))
    {
        return "Roll the dice";
    }
    if(std::holds_alternative<AwaitPath>(phase))
    {
        return "Choose a Path";
    }
    if(std::holds_alternative<AwaitPurchase>(phase))
    {
        return "Buy shares or decline";
    }
    if(std::holds_alternative<InAuction>(phase))
    {
        return "Bid or pass";
    }
    if(std::holds_alternative<AwaitTradeResponse>(phase))
    {
        return "Respond to Trade";
    }
    if(std::holds_alternative<Manage>(phase))
    {
        return "Manage your holdings";
    }
    if(std::holds_alternative<GameOver>(phase))
    {
        return "Game Over";
    }
    return {};
}

std::size_t HudModel::noteCount() const noexcept
{
    return notes_.size();
}

void HudModel::render(sgl::RenderQueue& queue, const Match& match, sgl::FontId font, const Theme& theme) const
{
    const GameState& state = match.state();
    float depth = 0.f;

    queue.push(sgl::Layer::Hud,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = phasePrompt(state.phase),
                   .size = 22,
                   .position = sgl::Vec2f{24.f, 16.f},
                   .color = sgl::Color{240, 240, 245},
               });

    for(int i = 0; i < state.seats; ++i)
    {
        const Money cash = state.cash[static_cast<std::size_t>(i)];
        const sgl::Color color = theme.pawn[static_cast<std::size_t>(i)];
        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::TextCmd{
                       .font = font,
                       .text = "P" + std::to_string(i) + " $" + std::to_string(cash.v),
                       .size = 18,
                       .position = sgl::Vec2f{24.f, 52.f + static_cast<float>(i) * 28.f},
                       .color = color,
                   });
    }

    constexpr float kBarX0 = 1040.f;
    constexpr float kBarYBase = 640.f;
    constexpr float kBarW = 28.f;
    constexpr float kBarGap = 12.f;
    for(std::size_t i = 0; i < state.market.index.size(); ++i)
    {
        const float t = std::clamp(static_cast<float>(state.market.index[i].v) / 200.f, 0.f, 1.f);
        const float h = 50.f + t * 150.f;
        const float x = kBarX0 + static_cast<float>(i) * (kBarW + kBarGap);
        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::RectCmd{
                       .rect = sgl::Rect<float>{.pos = {x, kBarYBase - h}, .size = {kBarW, h}},
                       .fill = theme.sector[i],
                   });
    }

    float noteY = 200.f;
    for(const HudNote& note: notes_)
    {
        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::TextCmd{
                       .font = font,
                       .text = note.text,
                       .size = 16,
                       .position = sgl::Vec2f{24.f, noteY},
                       .color = sgl::Color{220, 220, 200},
                   });
        noteY += 22.f;
    }
}

} // namespace sgl::syndicate
