#include <syndicate/app/Describe.hpp>

#include <cstddef>
#include <cstdint>
#include <sgl/core/Overloaded.hpp>
#include <sstream>
#include <string>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Economy.hpp>
#include <syndicate/sim/Events.hpp>
#include <syndicate/sim/Market.hpp>
#include <utility>
#include <variant>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] std::string withCost(std::string text, Money cost)
{
    text += " — Cost: ";
    text += formatMoney(cost);
    return text;
}

[[nodiscard]] std::string withGain(std::string text, Money gain)
{
    text += " — Gain: ";
    text += formatMoney(gain);
    return text;
}

[[nodiscard]] NodeId purchaseTile(const GameState& state) noexcept
{
    if(const auto* purch = std::get_if<AwaitPurchase>(&state.phase))
    {
        return purch->tile;
    }
    return NodeId{0};
}

[[nodiscard]] std::string_view sectorNameOf(const GameState& state, NodeId tile)
{
    if(const auto* sector = std::get_if<SectorTile>(&state.board.node(tile).kind))
    {
        return sectorName(sector->sector);
    }
    return "Unknown";
}

} // namespace

std::string formatMoney(Money money)
{
    if(money.v == 0)
    {
        return "$0";
    }

    const bool negative = money.v < 0;
    const std::int64_t absV = negative ? -money.v : money.v;

    std::ostringstream out;
    if(negative)
    {
        out << '-';
    }

    if(absV < 1000)
    {
        out << '$' << absV << 'k';
        return out.str();
    }

    const std::int64_t millions = absV / 1000;
    const std::int64_t tenths = (absV % 1000) / 100;
    out << '$' << millions;
    if(tenths != 0)
    {
        out << '.' << tenths;
    }
    out << 'M';
    return out.str();
}

std::string_view sectorName(Sector sector)
{
    switch(sector)
    {
    case Sector::AI:
        return "AI";
    case Sector::CleanEnergy:
        return "Clean Energy";
    case Sector::Biotech:
        return "Biotech";
    case Sector::CyberSec:
        return "CyberSec";
    case Sector::Logistics:
        return "Logistics";
    case Sector::Fintech:
        return "Fintech";
    }
    return "Unknown";
}

std::string describeCard(CardKind kind)
{
    switch(kind)
    {
    case CardKind::HostileTakeover:
        return "Hostile Takeover";
    case CardKind::Audit:
        return "Audit";
    case CardKind::MarketCrash:
        return "Market Crash";
    case CardKind::ShortSqueeze:
        return "Short Squeeze";
    case CardKind::Lobbying:
        return "Lobbying";
    case CardKind::Embargo:
        return "Embargo";
    case CardKind::Subsidy:
        return "Subsidy";
    case CardKind::Insider:
        return "Insider";
    }
    return "Card";
}

std::string_view cardEffectHint(CardKind kind)
{
    switch(kind)
    {
    case CardKind::HostileTakeover:
        return "Buy 1 share from a rival";
    case CardKind::Audit:
        return "Rival pays a tax";
    case CardKind::MarketCrash:
        return "Drop a sector index";
    case CardKind::ShortSqueeze:
        return "Spike a sector index";
    case CardKind::Lobbying:
        return "Free upgrade on a tile";
    case CardKind::Embargo:
        return "Lock a tile briefly";
    case CardKind::Subsidy:
        return "Cash from the bank";
    case CardKind::Insider:
        return "Peek next market event";
    }
    return "Play for an effect";
}

std::string describeTile(const GameState& state, NodeId tile)
{
    const TileNode& node = state.board.node(tile);
    return std::visit(
        sgl::Overloaded{
            [&](const SectorTile& sector) {
                std::ostringstream out;
                out << sectorName(sector.sector) << " Lab";
                out << " — price " << formatMoney(sharePrice(state.board, state.market, tile));
                const auto& h = state.ownership.holdings(tile);
                out << ", level " << static_cast<int>(h.level);
                out << ", bank " << static_cast<int>(h.bank);
                for(int i = 0; i < state.seats; ++i)
                {
                    out << ", P" << (i + 1) << '=' << static_cast<int>(h.shares[static_cast<std::size_t>(i)]);
                }
                const FeeSplit fee =
                    landingFee(state.board, state.ownership, state.market, tile, state.acting);
                out << ", fee " << formatMoney(fee.total);
                return out.str();
            },
            [](const Payday&) { return std::string{"Payday"}; },
            [](const Exchange&) { return std::string{"Exchange"}; },
            [](const Hub&) { return std::string{"Hub"}; },
            [](const AuditTile&) { return std::string{"Audit"}; },
            [](const Grant&) { return std::string{"Grant"}; },
        },
        node.kind);
}

namespace
{

[[nodiscard]] std::string playerTag(PlayerId id)
{
    return "P" + std::to_string(static_cast<int>(std::to_underlying(id)) + 1);
}

[[nodiscard]] std::string shortTileName(const GameState& state, NodeId tile)
{
    return std::visit(
        sgl::Overloaded{
            [&](const SectorTile& sector) { return std::string{sectorName(sector.sector)} + " Lab"; },
            [](const Payday&) { return std::string{"Payday"}; },
            [](const Exchange&) { return std::string{"Exchange"}; },
            [](const Hub&) { return std::string{"Hub"}; },
            [](const AuditTile&) { return std::string{"Audit"}; },
            [](const Grant&) { return std::string{"Grant"}; },
        },
        state.board.node(tile).kind);
}

} // namespace

std::string describeEvent(const GameState& state, const GameEvent& event)
{
    return std::visit(
        sgl::Overloaded{
            [](const Rolled& e) {
                std::ostringstream out;
                out << playerTag(e.player) << " rolled " << e.roll;
                return out.str();
            },
            [&](const Moved& e) {
                std::ostringstream out;
                out << playerTag(e.player) << " moved to " << shortTileName(state, e.to);
                return out.str();
            },
            [](const PaydayPaid& e) {
                std::ostringstream out;
                out << playerTag(e.player) << " payday " << formatMoney(e.salary) << " salary, "
                    << formatMoney(e.dividends) << " dividends, " << formatMoney(e.interest) << " interest";
                return out.str();
            },
            [&](const FeePaid& e) {
                std::ostringstream out;
                out << playerTag(e.payer) << " paid " << formatMoney(e.total) << " fee on "
                    << shortTileName(state, e.tile);
                return out.str();
            },
            [&](const Liquidated& e) {
                std::ostringstream out;
                out << playerTag(e.player) << " liquidated " << static_cast<int>(e.count) << " shares of "
                    << shortTileName(state, e.tile) << " for " << formatMoney(e.proceeds);
                return out.str();
            },
            [&](const SharesBought& e) {
                std::ostringstream out;
                out << playerTag(e.player) << " bought " << static_cast<int>(e.count) << " shares of "
                    << shortTileName(state, e.tile) << " for " << formatMoney(e.cost);
                return out.str();
            },
            [&](const AuctionDone& e) {
                std::ostringstream out;
                out << playerTag(e.winner) << " won " << static_cast<int>(e.shares) << " shares of "
                    << shortTileName(state, e.tile) << " for " << formatMoney(e.price);
                return out.str();
            },
            [](const CardPlayed& e) {
                std::ostringstream out;
                out << playerTag(e.player) << " played " << describeCard(e.kind);
                return out.str();
            },
            [](const TradeResolved& e) {
                std::ostringstream out;
                out << playerTag(e.from) << " trade with " << playerTag(e.to)
                    << (e.accepted ? " accepted" : " rejected");
                return out.str();
            },
            [](const MarketShifted& e) {
                std::ostringstream out;
                out << sectorName(e.sector) << ' ';
                if(e.delta >= 0)
                {
                    out << '+';
                }
                out << e.delta;
                return out.str();
            },
            [](const GameEnded&) { return std::string{"Game over"}; },
        },
        event);
}

Label describeCommand(const GameState& state, PlayerId /*actor*/, const Command& command)
{
    return std::visit(
        sgl::Overloaded{
            [](const RollDice&) {
                return Label{.text = "Roll", .tooltip = "Roll two dice to move."};
            },
            [](const BuyMomentum& c) {
                const char* delta = c.delta > 0 ? "+1" : "-1";
                std::string text = std::string{"Momentum "} + delta;
                text = withCost(std::move(text), Money{25});
                return Label{.text = std::move(text),
                             .tooltip = "Adjust your roll by one for $25k."};
            },
            [&](const ChooseDestination& c) {
                std::string text = "Go to ";
                text += describeTile(state, c.destination);
                return Label{.text = std::move(text),
                             .tooltip = "Move to the selected destination on the board."};
            },
            [&](const BuyShares& c) {
                const NodeId tile = purchaseTile(state);
                const auto name = sectorNameOf(state, tile);
                const int pct = static_cast<int>(c.count) * 10;
                std::ostringstream text;
                if(c.count == 1)
                {
                    text << "Buy 1 share of " << name << " (" << pct << "%)";
                }
                else
                {
                    text << "Buy " << static_cast<int>(c.count) << " shares of " << name << " (" << pct << "%)";
                }
                const Money unit = sharePrice(state.board, state.market, tile);
                const Money cost = unit * static_cast<int>(c.count);
                std::string body = withCost(text.str(), cost);
                std::ostringstream tip;
                tip << "Buy " << static_cast<int>(c.count) << " share"
                    << (c.count == 1 ? "" : "s") << " of " << name << " for " << formatMoney(cost) << '.';
                return Label{.text = std::move(body), .tooltip = tip.str()};
            },
            [](const DeclinePurchase&) {
                return Label{.text = "Decline", .tooltip = "Decline buying shares on this tile."};
            },
            [](const Bid& c) {
                std::string text = "Bid ";
                text += formatMoney(c.amount);
                return Label{.text = std::move(text),
                             .tooltip = "Place an auction bid of " + formatMoney(c.amount) + '.'};
            },
            [](const PassBid&) {
                return Label{.text = "Pass", .tooltip = "Pass on this auction round."};
            },
            [](const PlayCard& c) {
                const std::string card = describeCard(c.kind);
                return Label{.text = "Play card: " + card,
                             .tooltip = "Play your " + card + " card."};
            },
            [](const ProposeTrade&) {
                return Label{.text = "Propose trade", .tooltip = "Propose a trade to another player."};
            },
            [](const RespondTrade& c) {
                if(c.accept)
                {
                    return Label{.text = "Accept trade", .tooltip = "Accept the pending trade offer."};
                }
                return Label{.text = "Reject trade", .tooltip = "Reject the pending trade offer."};
            },
            [&](const Upgrade& c) {
                const auto& h = state.ownership.holdings(c.tile);
                const Money cost = upgradeCost(state.board, c.tile, h.level);
                const auto name = sectorNameOf(state, c.tile);
                std::string text = withCost(std::string{"Upgrade "} + std::string{name}, cost);
                return Label{.text = std::move(text),
                             .tooltip = "Upgrade " + std::string{name} + " for " + formatMoney(cost) + '.'};
            },
            [&](const SellShares& c) {
                const Money unit = sharePrice(state.board, state.market, c.tile);
                const Money gain = unit * static_cast<int>(c.count);
                const auto name = sectorNameOf(state, c.tile);
                std::ostringstream text;
                text << "Sell " << static_cast<int>(c.count) << " " << name;
                std::string body = withGain(text.str(), gain);
                return Label{.text = std::move(body),
                             .tooltip = "Sell shares of " + std::string{name} + " for " + formatMoney(gain) + '.'};
            },
            [](const BuyCard&) {
                constexpr Money kCost{100};
                std::string text = withCost("Buy card", kCost);
                return Label{.text = std::move(text),
                             .tooltip = "Buy a card from the Exchange for " + formatMoney(kCost) + '.'};
            },
            [](const EndTurn&) {
                return Label{.text = "End turn", .tooltip = "End your turn and pass play."};
            },
        },
        command);
}

} // namespace sgl::syndicate
