#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <sstream>
#include <string>
#include <syndicate/app/Describe.hpp>
#include <syndicate/app/HudModel.hpp>
#include <syndicate/app/TrendChart.hpp>
#include <syndicate/sim/Economy.hpp>
#include <utility>
#include <variant>

namespace sgl::syndicate
{

void HudModel::pushNote(std::string text, float ttl)
{
    notes_.push_back(HudNote{.text = std::move(text), .ttl = ttl});
}

void HudModel::recordMarket(const Market& market)
{
    history_.record(market);
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

std::vector<sgl::Hotspot> HudModel::chartHotspots() const
{
    return sgl::syndicate::chartHotspots(history_, marketChartRect());
}

std::string HudModel::playerTooltip(const Match& match, int seatIndex) const
{
    const GameState& state = match.state();
    if(seatIndex < 0 || seatIndex >= state.seats)
    {
        return {};
    }
    const PlayerId id{static_cast<std::uint8_t>(seatIndex)};
    const Money cash = state.cash[static_cast<std::size_t>(seatIndex)];
    const Money nw = netWorth(state.board, state.ownership, state.market, id, cash);
    int shareTiles = 0;
    int shareCount = 0;
    int controlled = 0;
    for(std::size_t t = 0; t < state.board.size(); ++t)
    {
        const NodeId tile{static_cast<std::uint16_t>(t)};
        const auto& h = state.ownership.holdings(tile);
        const std::uint8_t shares = h.shares[static_cast<std::size_t>(seatIndex)];
        if(shares > 0)
        {
            ++shareTiles;
            shareCount += shares;
        }
        if(const auto ctrl = state.ownership.controller(tile); ctrl && *ctrl == id)
        {
            ++controlled;
        }
    }
    std::ostringstream out;
    out << "Cash " << formatMoney(cash) << "  NW " << formatMoney(nw) << "  Shares " << shareCount << " on "
        << shareTiles << " tiles  Ctrl " << controlled;
    return out.str();
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

    {
        std::ostringstream progress;
        progress << "R" << state.round << "/" << state.config.maxRounds << "  target "
                 << formatMoney(state.config.targetNetWorth);
        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::TextCmd{
                       .font = font,
                       .text = progress.str(),
                       .size = 13,
                       .position = sgl::Vec2f{24.f, 42.f},
                       .color = sgl::Color{170, 180, 200},
                   });
    }

    const PlayerId acting = match.actingPlayer();
    for(int i = 0; i < state.seats; ++i)
    {
        const Money cash = state.cash[static_cast<std::size_t>(i)];
        const PlayerId id{static_cast<std::uint8_t>(i)};
        const Money nw = netWorth(state.board, state.ownership, state.market, id, cash);
        const sgl::Color color = theme.pawn[static_cast<std::size_t>(i)];
        const sgl::Rect<float> panel = playerPanelRect(i);
        const bool isActing = id == acting;
        const bool inDebt = cash.v < 0;

        sgl::Color fill = isActing ? sgl::Color{56, 64, 88, 230} : sgl::Color{36, 42, 58, 210};
        if(inDebt)
        {
            fill = sgl::Color{72, 36, 44, 230};
        }
        queue.push(sgl::Layer::Hud, depth++, sgl::roundedRect(panel, 6.f, fill));
        if(isActing)
        {
            queue.push(sgl::Layer::Hud, depth++,
                       sgl::roundedRectOutline(panel, 6.f, 2.f, sgl::Color{220, 230, 255, 240}));
        }
        else
        {
            queue.push(sgl::Layer::Hud, depth++, sgl::roundedRectOutline(panel, 6.f, 1.f, color));
        }

        queue.push(sgl::Layer::Hud,
                   depth++,
                   sgl::TextCmd{
                       .font = font,
                       .text = "P" + std::to_string(i + 1) + " " + formatMoney(cash) + "  NW " + formatMoney(nw),
                       .size = 15,
                       .position = sgl::Vec2f{panel.pos.x + 10.f, panel.pos.y + 4.f},
                       .color = inDebt ? sgl::Color{255, 160, 160} : color,
                   });

        const float target = static_cast<float>(std::max<std::int64_t>(1, state.config.targetNetWorth.v));
        const float ratio = std::clamp(static_cast<float>(nw.v) / target, 0.f, 1.f);
        const sgl::Rect<float> barBg{.pos = {panel.pos.x + 10.f, panel.pos.y + panel.size.y - 10.f},
                                     .size = {panel.size.x - 20.f, 4.f}};
        const sgl::Rect<float> barFg{.pos = barBg.pos, .size = {barBg.size.x * ratio, barBg.size.y}};
        queue.push(sgl::Layer::Hud, depth++, sgl::roundedRect(barBg, 2.f, sgl::Color{20, 24, 36, 220}));
        queue.push(sgl::Layer::Hud, depth++, sgl::roundedRect(barFg, 2.f, color));
    }

    renderTrendChart(queue, history_, marketChartRect(), theme, font);

    float noteY = 220.f;
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
