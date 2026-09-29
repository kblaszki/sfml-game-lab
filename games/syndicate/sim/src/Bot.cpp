#include <syndicate/sim/Bot.hpp>

#include <cstddef>
#include <cstdint>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Economy.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <syndicate/sim/Pathing.hpp>
#include <type_traits>
#include <variant>
#include <vector>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] std::size_t playerIndex(PlayerId id) noexcept
{
    return static_cast<std::size_t>(id);
}

[[nodiscard]] int pathSteps(const MoveOption& opt) noexcept
{
    if(opt.path.empty())
    {
        return 0;
    }
    return static_cast<int>(opt.path.size() - 1);
}

[[nodiscard]] const MoveOption* findOption(const AwaitPath& path, const ChooseDestination& dest)
{
    for(const MoveOption& opt: path.options)
    {
        if(opt.destination == dest.destination && opt.passesPayday == dest.passesPayday)
        {
            return &opt;
        }
    }
    return nullptr;
}

[[nodiscard]] int scoreCommand(const Match& match, PlayerId self, const BotPersona& persona, const Command& cmd)
{
    const GameState& s = match.state();
    const Money cash = s.cash[playerIndex(self)];

    return std::visit(
        [&](const auto& c) -> int {
            using T = std::decay_t<decltype(c)>;
            if constexpr(std::is_same_v<T, EndTurn>)
            {
                return 0;
            }
            else if constexpr(std::is_same_v<T, RollDice>)
            {
                return 100;
            }
            else if constexpr(std::is_same_v<T, ChooseDestination>)
            {
                int score = 0;
                if(const auto* pathPhase = std::get_if<AwaitPath>(&s.phase))
                {
                    if(const MoveOption* opt = findOption(*pathPhase, c))
                    {
                        score += 10 * pathSteps(*opt);
                    }
                }
                if(std::holds_alternative<SectorTile>(s.board.node(c.destination).kind))
                {
                    if(s.ownership.freeShares(c.destination) > 0 && cash > persona.cashReserve)
                    {
                        score += 30;
                    }
                }
                const FeeSplit fee = landingFee(s.board, s.ownership, s.market, c.destination, self);
                if(fee.total > Money{cash.v / 2})
                {
                    score -= 20;
                }
                if(c.passesPayday)
                {
                    score += 15;
                }
                return score;
            }
            else if constexpr(std::is_same_v<T, BuyShares>)
            {
                const auto* purch = std::get_if<AwaitPurchase>(&s.phase);
                if(!purch)
                {
                    return -100;
                }
                const Money unit = sharePrice(s.board, s.market, purch->tile);
                const Money cost = unit * static_cast<int>(c.count);
                if(cash - cost >= persona.cashReserve && unit > Money{0})
                {
                    return 40 + persona.riskAppetite.v / 10;
                }
                return -100;
            }
            else if constexpr(std::is_same_v<T, Bid>)
            {
                if(!(c.amount > cash - persona.cashReserve))
                {
                    return 20;
                }
                return -100;
            }
            else if constexpr(std::is_same_v<T, PassBid>)
            {
                return 5;
            }
            else if constexpr(std::is_same_v<T, PlayCard>)
            {
                return 25;
            }
            else if constexpr(std::is_same_v<T, DeclinePurchase>)
            {
                return 1;
            }
            else if constexpr(std::is_same_v<T, Upgrade>)
            {
                const auto& h = s.ownership.holdings(c.tile);
                const Money cost = upgradeCost(s.board, c.tile, h.level);
                if(!(cash < cost))
                {
                    return 35;
                }
                return 0;
            }
            else if constexpr(std::is_same_v<T, BuyMomentum>)
            {
                return 10;
            }
            else
            {
                return 0;
            }
        },
        cmd);
}

} // namespace

Command chooseCommand(const Match& match, PlayerId self, const BotPersona& persona)
{
    const std::vector<Command> cmds = match.legalCommands(self);
    if(cmds.empty())
    {
        return EndTurn{};
    }

    int bestScore = scoreCommand(match, self, persona, cmds[0]);
    std::size_t best = 0;
    for(std::size_t i = 1; i < cmds.size(); ++i)
    {
        const int score = scoreCommand(match, self, persona, cmds[i]);
        if(score > bestScore)
        {
            bestScore = score;
            best = i;
        }
    }
    return cmds[best];
}

} // namespace sgl::syndicate
