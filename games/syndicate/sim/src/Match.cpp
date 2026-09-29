#include <syndicate/sim/Match.hpp>

#include <algorithm>
#include <cstddef>
#include <span>
#include <syndicate/sim/BoardLayouts.hpp>
#include <syndicate/sim/Economy.hpp>
#include <type_traits>
#include <utility>
#include <variant>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] std::size_t idx(PlayerId p) noexcept
{
    return static_cast<std::size_t>(std::to_underlying(p));
}

[[nodiscard]] PlayerId pid(std::size_t i) noexcept
{
    return PlayerId{static_cast<std::uint8_t>(i)};
}

[[nodiscard]] PlayerId nextSeat(PlayerId p, int seats) noexcept
{
    return pid((idx(p) + 1) % static_cast<std::size_t>(seats));
}

[[nodiscard]] bool isGameOver(const Phase& phase) noexcept
{
    return std::holds_alternative<GameOver>(phase);
}

[[nodiscard]] bool underEmbargo(const GameState& s, NodeId tile) noexcept
{
    for(const auto& [n, until]: s.embargoUntilRound)
    {
        if(n == tile && until >= s.round)
        {
            return true;
        }
    }
    return false;
}

void fillMarketDeck(GameState& s)
{
    s.marketDeck.clear();
    s.marketDeck.reserve(12);
    constexpr Sector sectors[] = {Sector::AI,
                                  Sector::CleanEnergy,
                                  Sector::Biotech,
                                  Sector::CyberSec,
                                  Sector::Logistics,
                                  Sector::Fintech};
    for(Sector sector: sectors)
    {
        s.marketDeck.push_back(MarketEventCard{sector, +20});
        s.marketDeck.push_back(MarketEventCard{sector, -20});
    }
    for(std::size_t i = s.marketDeck.size(); i > 1; --i)
    {
        const std::size_t j = static_cast<std::size_t>(s.rng.uniformBelow(static_cast<std::uint32_t>(i)));
        std::swap(s.marketDeck[i - 1], s.marketDeck[j]);
    }
}

[[nodiscard]] MarketEventCard drawMarketEvent(GameState& s)
{
    if(s.revealed)
    {
        const MarketEventCard card = *s.revealed;
        s.revealed.reset();
        // Peek left the card on top; consume it now.
        if(!s.marketDeck.empty() && s.marketDeck.back().sector == card.sector &&
           s.marketDeck.back().delta == card.delta)
        {
            s.marketDeck.pop_back();
        }
        return card;
    }
    if(s.marketDeck.empty())
    {
        fillMarketDeck(s);
    }
    const MarketEventCard card = s.marketDeck.back();
    s.marketDeck.pop_back();
    return card;
}

void peekMarketEvent(GameState& s)
{
    if(s.marketDeck.empty())
    {
        fillMarketDeck(s);
    }
    if(!s.marketDeck.empty())
    {
        s.revealed = s.marketDeck.back(); // do not consume
    }
}

[[nodiscard]] std::uint8_t actorSharesInSector(const GameState& s, PlayerId actor, Sector sector)
{
    std::uint8_t total = 0;
    for(std::size_t i = 0; i < s.board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        if(const auto* tile = std::get_if<SectorTile>(&s.board.node(id).kind))
        {
            if(tile->sector == sector)
            {
                total = static_cast<std::uint8_t>(total + s.ownership.holdings(id).shares[idx(actor)]);
            }
        }
    }
    return total;
}

void emit(GameState& s, GameEvent e)
{
    s.events.push_back(e);
}

void payFee(GameState& s, PlayerId payer, NodeId tile)
{
    FeeSplit fee = landingFee(s.board, s.ownership, s.market, tile, payer);
    if(underEmbargo(s, tile))
    {
        fee.total = Money{0};
        fee.toPlayer = {};
    }

    Money& cash = s.cash[idx(payer)];
    if(cash < fee.total)
    {
        const Money shortfall = cash.v > 0 ? fee.total - cash : fee.total;
        const auto plan = liquidationPlan(s.board, s.ownership, s.market, payer, shortfall);
        for(const Sale& sale: plan)
        {
            auto& h = s.ownership.holdings(sale.tile);
            h.shares[idx(payer)] = static_cast<std::uint8_t>(h.shares[idx(payer)] - sale.count);
            h.bank = static_cast<std::uint8_t>(h.bank + sale.count);
            const Money price = sharePrice(s.board, s.market, sale.tile);
            const Money proceeds = scale(price, Percent{80}) * static_cast<int>(sale.count);
            cash = cash + proceeds;
            emit(s, Liquidated{payer, sale.tile, sale.count, proceeds});
        }
    }

    cash = cash - fee.total;
    for(std::size_t i = 0; i < kMaxPlayers; ++i)
    {
        if(fee.toPlayer[i].v != 0)
        {
            s.cash[i] = s.cash[i] + fee.toPlayer[i];
        }
    }
    emit(s, FeePaid{payer, tile, fee.total});
}

void enterManage(GameState& s, PlayerId who)
{
    s.acting = who;
    s.turnOwner = who;
    s.phase = Manage{0, 0};
}

void resolveLanding(GameState& s, PlayerId who, NodeId dest)
{
    const TileKind& kind = s.board.node(dest).kind;

    if(std::holds_alternative<SectorTile>(kind))
    {
        const std::uint8_t free = s.ownership.freeShares(dest);
        if(free > 0)
        {
            const std::uint8_t maxCount = static_cast<std::uint8_t>(std::min<int>(3, free));
            s.phase = AwaitPurchase{dest, maxCount};
            s.acting = who;
            return;
        }
        payFee(s, who, dest);
        enterManage(s, who);
        return;
    }

    if(std::holds_alternative<Hub>(kind))
    {
        if(s.hands[idx(who)].cards().size() < Hand::kMax)
        {
            if(auto drawn = s.deck.draw(s.rng))
            {
                if(const auto added = s.hands[idx(who)].add(*drawn); !added)
                {
                    s.deck.discard(*drawn);
                }
            }
        }
        enterManage(s, who);
        return;
    }

    if(std::holds_alternative<Exchange>(kind))
    {
        enterManage(s, who);
        return;
    }

    if(const auto* grant = std::get_if<Grant>(&kind))
    {
        s.cash[idx(who)] = s.cash[idx(who)] + grant->amount;
        enterManage(s, who);
        return;
    }

    if(const auto* audit = std::get_if<AuditTile>(&kind))
    {
        Money& cash = s.cash[idx(who)];
        const Money base = cash.v > 0 ? cash : Money{0};
        const Money tax = scale(base, audit->tax);
        cash = cash - tax;
        enterManage(s, who);
        return;
    }

    // Payday destination: salary already paid via passesPayday.
    enterManage(s, who);
}

void applyPayday(GameState& s, PlayerId who)
{
    const Money salary{200};
    const Money divs = dividends(s.board, s.ownership, s.market, who);
    Money& cash = s.cash[idx(who)];
    cash = cash + salary + divs;
    Money interest{0};
    if(cash.v < 0)
    {
        interest = Money{(-cash.v) / 10};
        cash = cash - interest;
    }
    emit(s, PaydayPaid{who, salary, divs, interest});
}

[[nodiscard]] std::vector<RankingEntry> buildRanking(const GameState& s)
{
    std::vector<RankingEntry> ranking;
    ranking.reserve(static_cast<std::size_t>(s.seats));
    for(int i = 0; i < s.seats; ++i)
    {
        const PlayerId p = pid(static_cast<std::size_t>(i));
        ranking.push_back(RankingEntry{p,
                                       netWorth(s.board, s.ownership, s.market, p, s.cash[idx(p)]),
                                       s.cash[idx(p)]});
    }
    std::sort(ranking.begin(), ranking.end(), [](const RankingEntry& a, const RankingEntry& b) {
        if(a.net != b.net)
        {
            return a.net > b.net;
        }
        if(a.cash != b.cash)
        {
            return a.cash > b.cash;
        }
        return a.player < b.player;
    });
    return ranking;
}

void checkGameOver(GameState& s)
{
    bool over = s.round > s.config.maxRounds;
    if(!over)
    {
        for(int i = 0; i < s.seats; ++i)
        {
            const PlayerId p = pid(static_cast<std::size_t>(i));
            if(netWorth(s.board, s.ownership, s.market, p, s.cash[idx(p)]) >= s.config.targetNetWorth)
            {
                over = true;
                break;
            }
        }
    }
    if(over)
    {
        s.phase = GameOver{buildRanking(s)};
        emit(s, GameEnded{});
    }
}

void endOfRound(GameState& s)
{
    ++s.round;
    const MarketEventCard card = drawMarketEvent(s);
    applyShift(s.market, card.sector, card.delta);
    emit(s, MarketShifted{card.sector, card.delta});
    revertTowardCenter(s.market);
    checkGameOver(s);
}

void finishTurn(GameState& s)
{
    const PlayerId current = s.turnOwner;
    if(idx(current) + 1 >= static_cast<std::size_t>(s.seats))
    {
        endOfRound(s);
        if(isGameOver(s.phase))
        {
            return;
        }
        s.acting = PlayerId{0};
        s.turnOwner = PlayerId{0};
        s.phase = AwaitRoll{};
        return;
    }
    const PlayerId next = nextSeat(current, s.seats);
    s.acting = next;
    s.turnOwner = next;
    s.phase = AwaitRoll{};
}

void applyAuctionOutcome(GameState& s, const AuctionOutcome& outcome, NodeId tile, std::uint8_t shares)
{
    if(const auto* sold = std::get_if<Sold>(&outcome))
    {
        auto& h = s.ownership.holdings(tile);
        h.bank = static_cast<std::uint8_t>(h.bank - shares);
        h.shares[idx(sold->winner)] = static_cast<std::uint8_t>(h.shares[idx(sold->winner)] + shares);
        s.cash[idx(sold->winner)] = s.cash[idx(sold->winner)] - sold->price;
        emit(s, AuctionDone{tile, shares, true, sold->winner, sold->price});
        enterManage(s, s.turnOwner);
        return;
    }
    if(std::holds_alternative<Unsold>(outcome))
    {
        emit(s, AuctionDone{tile, shares, false, PlayerId{0}, Money{0}});
        enterManage(s, s.turnOwner);
        return;
    }
    // Continue: sync acting to toAct
    auto& auction = std::get<InAuction>(s.phase).auction;
    s.acting = auction.toAct;
}

[[nodiscard]] CommandError mapAuctionError(AuctionError e)
{
    switch(e)
    {
    case AuctionError::NotYourTurn:
    case AuctionError::NotActive:
        return CommandError::NotYourTurn;
    case AuctionError::CannotAfford:
        return CommandError::InsufficientFunds;
    case AuctionError::BidTooLow:
        return CommandError::InvalidArgument;
    case AuctionError::Closed:
        return CommandError::WrongPhase;
    }
    return CommandError::InvalidArgument;
}

[[nodiscard]] bool canAffordShareGrab(const GameState& s, PlayerId actor, const ShareGrab& grab)
{
    const Money price = scale(sharePrice(s.board, s.market, grab.tile), Percent{150});
    if(s.ownership.holdings(grab.tile).shares[idx(grab.from)] < 1)
    {
        return false;
    }
    return !(s.cash[idx(actor)] < price);
}

[[nodiscard]] bool canFreeUpgrade(const GameState& s, PlayerId actor, NodeId tile)
{
    const auto ctrl = s.ownership.controller(tile);
    if(!ctrl || *ctrl != actor)
    {
        return false;
    }
    return s.ownership.holdings(tile).level < 3;
}

[[nodiscard]] std::optional<CardTarget> dummyTargetFor(const GameState& s, PlayerId actor, CardKind kind)
{
    switch(kind)
    {
    case CardKind::Subsidy:
    case CardKind::Insider:
        return CardTarget{NoTarget{}};
    case CardKind::Audit:
        return CardTarget{PlayerTarget{PlayerId{0}}};
    case CardKind::MarketCrash:
    case CardKind::ShortSqueeze:
        return CardTarget{SectorTarget{Sector::AI}};
    case CardKind::Embargo:
        return CardTarget{TileTarget{NodeId{1}}};
    case CardKind::Lobbying:
        for(std::size_t i = 0; i < s.board.size(); ++i)
        {
            const NodeId id{static_cast<std::uint16_t>(i)};
            if(canFreeUpgrade(s, actor, id))
            {
                return CardTarget{TileTarget{id}};
            }
        }
        return std::nullopt;
    case CardKind::HostileTakeover:
        for(std::size_t ti = 0; ti < s.board.size(); ++ti)
        {
            const NodeId tile{static_cast<std::uint16_t>(ti)};
            if(!std::holds_alternative<SectorTile>(s.board.node(tile).kind))
            {
                continue;
            }
            const Money price = scale(sharePrice(s.board, s.market, tile), Percent{150});
            if(s.cash[idx(actor)] < price)
            {
                continue;
            }
            for(int p = 0; p < s.seats; ++p)
            {
                const PlayerId victim = pid(static_cast<std::size_t>(p));
                if(victim == actor)
                {
                    continue;
                }
                if(s.ownership.holdings(tile).shares[idx(victim)] >= 1)
                {
                    return CardTarget{TakeoverTarget{victim, tile}};
                }
            }
        }
        return std::nullopt;
    }
    return std::nullopt;
}

[[nodiscard]] std::expected<void, CommandError> applyCardEffects(GameState& s,
                                                                  PlayerId actor,
                                                                  CardKind kind,
                                                                  const std::vector<CardEffect>& effects)
{
    for(const CardEffect& effect: effects)
    {
        if(const auto* md = std::get_if<MarketDelta>(&effect))
        {
            applyShift(s.market, md->sector, md->delta);
            if(kind == CardKind::ShortSqueeze)
            {
                const auto shares = actorSharesInSector(s, actor, md->sector);
                s.cash[idx(actor)] = s.cash[idx(actor)] + Money{50} * static_cast<int>(shares);
            }
            continue;
        }
        if(const auto* cd = std::get_if<CashDelta>(&effect))
        {
            if(kind == CardKind::Subsidy)
            {
                s.cash[idx(actor)] = s.cash[idx(actor)] + Money{200};
            }
            else if(kind == CardKind::Audit)
            {
                Money& targetCash = s.cash[idx(cd->player)];
                const Money base = targetCash.v > 0 ? targetCash : Money{0};
                const Money tax = scale(base, Percent{10});
                targetCash = targetCash - tax;
            }
            continue;
        }
        if(const auto* grab = std::get_if<ShareGrab>(&effect))
        {
            const Money price = scale(sharePrice(s.board, s.market, grab->tile), Percent{150});
            if(s.ownership.holdings(grab->tile).shares[idx(grab->from)] < 1)
            {
                return std::unexpected(CommandError::InvalidArgument);
            }
            if(s.cash[idx(actor)] < price)
            {
                return std::unexpected(CommandError::InsufficientFunds);
            }
            auto& h = s.ownership.holdings(grab->tile);
            h.shares[idx(grab->from)] = static_cast<std::uint8_t>(h.shares[idx(grab->from)] - 1);
            h.shares[idx(actor)] = static_cast<std::uint8_t>(h.shares[idx(actor)] + 1);
            s.cash[idx(actor)] = s.cash[idx(actor)] - price;
            s.cash[idx(grab->from)] = s.cash[idx(grab->from)] + price;
            continue;
        }
        if(const auto* up = std::get_if<FreeUpgrade>(&effect))
        {
            if(!canFreeUpgrade(s, actor, up->tile))
            {
                return std::unexpected(CommandError::InvalidArgument);
            }
            ++s.ownership.holdings(up->tile).level;
            continue;
        }
        if(const auto* emb = std::get_if<EmbargoSet>(&effect))
        {
            s.embargoUntilRound.emplace_back(emb->tile, s.round + 1);
            continue;
        }
        if(std::holds_alternative<RevealNext>(effect))
        {
            peekMarketEvent(s);
            continue;
        }
    }
    return {};
}

void transferShares(GameState& s, PlayerId from, PlayerId to, NodeId tile, std::uint8_t count)
{
    auto& h = s.ownership.holdings(tile);
    h.shares[idx(from)] = static_cast<std::uint8_t>(h.shares[idx(from)] - count);
    h.shares[idx(to)] = static_cast<std::uint8_t>(h.shares[idx(to)] + count);
}

void applyTrade(GameState& s, const TradeOffer& offer)
{
    s.cash[idx(offer.from)] = s.cash[idx(offer.from)] - offer.cashFrom + offer.cashTo;
    s.cash[idx(offer.to)] = s.cash[idx(offer.to)] - offer.cashTo + offer.cashFrom;
    for(const ShareLot& lot: offer.sharesFrom)
    {
        transferShares(s, offer.from, offer.to, lot.tile, lot.count);
    }
    for(const ShareLot& lot: offer.sharesTo)
    {
        transferShares(s, offer.to, offer.from, lot.tile, lot.count);
    }
}

[[nodiscard]] auto sharesOfLambda(const GameState& s)
{
    return [&s](PlayerId who, NodeId tile) -> std::uint8_t {
        return s.ownership.holdings(tile).shares[idx(who)];
    };
}

} // namespace

Match::Match(GameState state)
    : state_(std::move(state))
{
}

std::expected<Match, SetupError> Match::create(const MatchConfig& cfg)
{
    if(cfg.seats < 2 || cfg.seats > 4)
    {
        return std::unexpected(SetupError::BadSeats);
    }

    BoardGraph board = standardBoard();
    GameState s{
        .board = std::move(board),
        .ownership = Ownership{0},
        .market = Market{},
        .deck = Deck{},
        .hands = {},
        .cash = {},
        .position = {},
        .cameFrom = {},
        .phase = AwaitRoll{},
        .acting = PlayerId{0},
        .turnOwner = PlayerId{0},
        .round = 1,
        .seats = cfg.seats,
        .rng = sgl::Pcg32{cfg.seed},
        .events = {},
        .config = cfg,
        .embargoUntilRound = {},
        .marketDeck = {},
        .revealed = std::nullopt,
    };
    s.ownership = Ownership{s.board.size()};
    for(int i = 0; i < cfg.seats; ++i)
    {
        s.cash[static_cast<std::size_t>(i)] = cfg.startCash;
        s.position[static_cast<std::size_t>(i)] = NodeId{0};
        s.cameFrom[static_cast<std::size_t>(i)] = std::nullopt;
    }
    for(std::size_t i = 0; i < 6; ++i)
    {
        s.market.index[i] = Percent{100};
    }
    s.deck.fillAndShuffle(s.rng);
    fillMarketDeck(s);
    return Match{std::move(s)};
}

PlayerId Match::actingPlayer() const noexcept
{
    return state_.acting;
}

const GameState& Match::state() const noexcept
{
    return state_;
}

std::vector<GameEvent> Match::drainEvents()
{
    std::vector<GameEvent> out;
    out.swap(state_.events);
    return out;
}

std::expected<void, CommandError> Match::apply(PlayerId actor, const Command& cmd)
{
    if(isGameOver(state_.phase))
    {
        return std::unexpected(CommandError::WrongPhase);
    }
    if(actor != state_.acting)
    {
        return std::unexpected(CommandError::NotYourTurn);
    }

    return std::visit(
        [&](const auto& c) -> std::expected<void, CommandError> {
            using T = std::decay_t<decltype(c)>;

            if constexpr(std::is_same_v<T, RollDice>)
            {
                auto* phase = std::get_if<AwaitRoll>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                const int d1 = static_cast<int>(state_.rng.nextU32() % 4u) + 1;
                const int d2 = static_cast<int>(state_.rng.nextU32() % 4u) + 1;
                const int roll = d1 + d2;
                emit(state_, Rolled{actor, roll});
                auto options = moveOptions(state_.board, state_.position[idx(actor)], state_.cameFrom[idx(actor)], roll);
                state_.phase = AwaitPath{std::move(options), roll, false};
                return {};
            }
            else if constexpr(std::is_same_v<T, BuyMomentum>)
            {
                auto* phase = std::get_if<AwaitPath>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                if(phase->momentumUsed || (c.delta != 1 && c.delta != -1))
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                constexpr Money kCost{25};
                if(state_.cash[idx(actor)] < kCost)
                {
                    return std::unexpected(CommandError::InsufficientFunds);
                }
                state_.cash[idx(actor)] = state_.cash[idx(actor)] - kCost;
                const int newRoll = std::max(1, phase->roll + c.delta);
                phase->roll = newRoll;
                phase->momentumUsed = true;
                phase->options =
                    moveOptions(state_.board, state_.position[idx(actor)], state_.cameFrom[idx(actor)], newRoll);
                return {};
            }
            else if constexpr(std::is_same_v<T, ChooseDestination>)
            {
                auto* phase = std::get_if<AwaitPath>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                const MoveOption* chosen = nullptr;
                for(const MoveOption& opt: phase->options)
                {
                    if(opt.destination == c.destination && opt.passesPayday == c.passesPayday)
                    {
                        chosen = &opt;
                        break;
                    }
                }
                if(!chosen)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                const NodeId from = state_.position[idx(actor)];
                const NodeId dest = chosen->destination;
                // path includes start; cameFrom = node before end
                if(chosen->path.size() >= 2)
                {
                    state_.cameFrom[idx(actor)] = chosen->path[chosen->path.size() - 2];
                }
                else
                {
                    state_.cameFrom[idx(actor)] = from;
                }
                state_.position[idx(actor)] = dest;
                emit(state_, Moved{actor, from, dest, chosen->passesPayday});
                if(chosen->passesPayday)
                {
                    applyPayday(state_, actor);
                }
                resolveLanding(state_, actor, dest);
                return {};
            }
            else if constexpr(std::is_same_v<T, BuyShares>)
            {
                auto* phase = std::get_if<AwaitPurchase>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                if(c.count < 1 || c.count > phase->maxCount)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                const Money unit = sharePrice(state_.board, state_.market, phase->tile);
                const Money cost = unit * static_cast<int>(c.count);
                if(state_.cash[idx(actor)] < cost)
                {
                    return std::unexpected(CommandError::InsufficientFunds);
                }
                auto& h = state_.ownership.holdings(phase->tile);
                h.bank = static_cast<std::uint8_t>(h.bank - c.count);
                h.shares[idx(actor)] = static_cast<std::uint8_t>(h.shares[idx(actor)] + c.count);
                state_.cash[idx(actor)] = state_.cash[idx(actor)] - cost;
                emit(state_, SharesBought{actor, phase->tile, c.count, cost});
                enterManage(state_, actor);
                return {};
            }
            else if constexpr(std::is_same_v<T, DeclinePurchase>)
            {
                auto* phase = std::get_if<AwaitPurchase>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                const NodeId tile = phase->tile;
                const std::uint8_t free = state_.ownership.freeShares(tile);
                if(free >= 3)
                {
                    const Money block = sharePrice(state_.board, state_.market, tile) * 3;
                    const Money opening = scale(block, Percent{50});
                    std::array<bool, kMaxPlayers> eligible{};
                    for(int i = 0; i < state_.seats; ++i)
                    {
                        eligible[static_cast<std::size_t>(i)] = state_.cash[static_cast<std::size_t>(i)].v >= 0;
                    }
                    const PlayerId first = nextSeat(actor, state_.seats);
                    AuctionState auction = startAuction(tile, 3, opening, state_.seats, first, eligible);
                    state_.turnOwner = actor;
                    state_.phase = InAuction{auction};
                    state_.acting = std::get<InAuction>(state_.phase).auction.toAct;
                    return {};
                }
                enterManage(state_, actor);
                return {};
            }
            else if constexpr(std::is_same_v<T, Bid>)
            {
                auto* phase = std::get_if<InAuction>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                auto result = bid(phase->auction, actor, c.amount, state_.cash[idx(actor)]);
                if(!result)
                {
                    return std::unexpected(mapAuctionError(result.error()));
                }
                applyAuctionOutcome(state_, *result, phase->auction.tile, phase->auction.shares);
                return {};
            }
            else if constexpr(std::is_same_v<T, PassBid>)
            {
                auto* phase = std::get_if<InAuction>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                auto result = passBid(phase->auction, actor);
                if(!result)
                {
                    return std::unexpected(mapAuctionError(result.error()));
                }
                applyAuctionOutcome(state_, *result, phase->auction.tile, phase->auction.shares);
                return {};
            }
            else if constexpr(std::is_same_v<T, PlayCard>)
            {
                auto* phase = std::get_if<Manage>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                if(phase->cardsPlayed >= 1)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                auto effects = describeEffect(c.kind, c.target);
                if(!effects)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                // Pre-check effects that can fail so we do not discard on error.
                for(const CardEffect& effect: *effects)
                {
                    if(const auto* grab = std::get_if<ShareGrab>(&effect))
                    {
                        if(!canAffordShareGrab(state_, actor, *grab))
                        {
                            if(state_.ownership.holdings(grab->tile).shares[idx(grab->from)] < 1)
                            {
                                return std::unexpected(CommandError::InvalidArgument);
                            }
                            return std::unexpected(CommandError::InsufficientFunds);
                        }
                    }
                    if(const auto* up = std::get_if<FreeUpgrade>(&effect))
                    {
                        if(!canFreeUpgrade(state_, actor, up->tile))
                        {
                            return std::unexpected(CommandError::InvalidArgument);
                        }
                    }
                }
                if(!state_.hands[idx(actor)].remove(c.kind))
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                state_.deck.discard(c.kind);
                if(auto applied = applyCardEffects(state_, actor, c.kind, *effects); !applied)
                {
                    return applied;
                }
                ++phase->cardsPlayed;
                emit(state_, CardPlayed{actor, c.kind});
                return {};
            }
            else if constexpr(std::is_same_v<T, ProposeTrade>)
            {
                auto* phase = std::get_if<Manage>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                if(phase->tradesProposed >= 3)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                if(c.offer.from != actor)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                auto ok = validateTrade(c.offer, std::span<const Money>{state_.cash}, sharesOfLambda(state_));
                if(!ok)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                const int cardsPlayed = phase->cardsPlayed;
                const int tradesProposed = phase->tradesProposed + 1;
                state_.phase = AwaitTradeResponse{c.offer, cardsPlayed, tradesProposed};
                state_.acting = c.offer.to;
                return {};
            }
            else if constexpr(std::is_same_v<T, RespondTrade>)
            {
                auto* phase = std::get_if<AwaitTradeResponse>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                if(actor != phase->offer.to)
                {
                    return std::unexpected(CommandError::NotYourTurn);
                }
                const TradeOffer offer = phase->offer;
                const PlayerId owner = offer.from;
                const int cardsPlayed = phase->cardsPlayed;
                const int tradesProposed = phase->tradesProposed;
                if(c.accept)
                {
                    auto ok = validateTrade(offer, std::span<const Money>{state_.cash}, sharesOfLambda(state_));
                    if(!ok)
                    {
                        return std::unexpected(CommandError::InvalidArgument);
                    }
                    applyTrade(state_, offer);
                }
                emit(state_, TradeResolved{offer.from, offer.to, c.accept});
                state_.acting = owner;
                state_.turnOwner = owner;
                state_.phase = Manage{cardsPlayed, tradesProposed};
                return {};
            }
            else if constexpr(std::is_same_v<T, Upgrade>)
            {
                auto* phase = std::get_if<Manage>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                const auto ctrl = state_.ownership.controller(c.tile);
                if(!ctrl || *ctrl != actor)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                auto& h = state_.ownership.holdings(c.tile);
                if(h.level >= 3)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                const Money cost = upgradeCost(state_.board, c.tile, h.level);
                if(state_.cash[idx(actor)] < cost)
                {
                    return std::unexpected(CommandError::InsufficientFunds);
                }
                state_.cash[idx(actor)] = state_.cash[idx(actor)] - cost;
                ++h.level;
                return {};
            }
            else if constexpr(std::is_same_v<T, SellShares>)
            {
                auto* phase = std::get_if<Manage>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                auto& h = state_.ownership.holdings(c.tile);
                if(c.count < 1 || h.shares[idx(actor)] < c.count)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                const Money unit = sharePrice(state_.board, state_.market, c.tile);
                const Money proceeds = unit * static_cast<int>(c.count);
                h.shares[idx(actor)] = static_cast<std::uint8_t>(h.shares[idx(actor)] - c.count);
                h.bank = static_cast<std::uint8_t>(h.bank + c.count);
                state_.cash[idx(actor)] = state_.cash[idx(actor)] + proceeds;
                return {};
            }
            else if constexpr(std::is_same_v<T, BuyCard>)
            {
                auto* phase = std::get_if<Manage>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                const NodeId pos = state_.position[idx(actor)];
                if(!std::holds_alternative<Exchange>(state_.board.node(pos).kind))
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                constexpr Money kCost{100};
                if(state_.cash[idx(actor)] < kCost)
                {
                    return std::unexpected(CommandError::InsufficientFunds);
                }
                if(state_.hands[idx(actor)].cards().size() >= Hand::kMax)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                auto drawn = state_.deck.draw(state_.rng);
                if(!drawn)
                {
                    return std::unexpected(CommandError::InvalidArgument);
                }
                if(!state_.hands[idx(actor)].add(*drawn))
                {
                    state_.deck.discard(*drawn);
                    return std::unexpected(CommandError::InvalidArgument);
                }
                state_.cash[idx(actor)] = state_.cash[idx(actor)] - kCost;
                return {};
            }
            else if constexpr(std::is_same_v<T, EndTurn>)
            {
                auto* phase = std::get_if<Manage>(&state_.phase);
                if(!phase)
                {
                    return std::unexpected(CommandError::WrongPhase);
                }
                finishTurn(state_);
                return {};
            }
            else
            {
                return std::unexpected(CommandError::InvalidArgument);
            }
        },
        cmd);
}

std::vector<Command> Match::legalCommands(PlayerId actor) const
{
    std::vector<Command> out;
    if(actor != state_.acting || isGameOver(state_.phase))
    {
        return out;
    }

    if(std::holds_alternative<AwaitRoll>(state_.phase))
    {
        out.push_back(RollDice{});
        return out;
    }

    if(const auto* path = std::get_if<AwaitPath>(&state_.phase))
    {
        for(const MoveOption& opt: path->options)
        {
            out.push_back(ChooseDestination{opt.destination, opt.passesPayday});
        }
        if(!path->momentumUsed && !(state_.cash[idx(actor)] < Money{25}))
        {
            out.push_back(BuyMomentum{+1});
            out.push_back(BuyMomentum{-1});
        }
        return out;
    }

    if(const auto* purch = std::get_if<AwaitPurchase>(&state_.phase))
    {
        const Money unit = sharePrice(state_.board, state_.market, purch->tile);
        for(std::uint8_t n = 1; n <= purch->maxCount; ++n)
        {
            const Money cost = unit * static_cast<int>(n);
            if(!(state_.cash[idx(actor)] < cost))
            {
                out.push_back(BuyShares{n});
            }
        }
        out.push_back(DeclinePurchase{});
        return out;
    }

    if(const auto* auctionPhase = std::get_if<InAuction>(&state_.phase))
    {
        const AuctionState& a = auctionPhase->auction;
        if(a.toAct != actor)
        {
            return out;
        }
        out.push_back(PassBid{});
        Money minBid = a.highBid;
        if(a.leader)
        {
            minBid = a.highBid + Money{10};
        }
        if(!(state_.cash[idx(actor)] < minBid))
        {
            out.push_back(Bid{minBid});
            const Money plus50 = minBid + Money{50};
            if(!(state_.cash[idx(actor)] < plus50))
            {
                out.push_back(Bid{plus50});
            }
        }
        return out;
    }

    if(std::holds_alternative<AwaitTradeResponse>(state_.phase))
    {
        out.push_back(RespondTrade{true});
        out.push_back(RespondTrade{false});
        return out;
    }

    if(const auto* manage = std::get_if<Manage>(&state_.phase))
    {
        out.push_back(EndTurn{});

        for(std::size_t i = 0; i < state_.board.size(); ++i)
        {
            const NodeId tile{static_cast<std::uint16_t>(i)};
            const auto ctrl = state_.ownership.controller(tile);
            if(ctrl && *ctrl == actor)
            {
                const auto& h = state_.ownership.holdings(tile);
                if(h.level < 3)
                {
                    const Money cost = upgradeCost(state_.board, tile, h.level);
                    if(!(state_.cash[idx(actor)] < cost))
                    {
                        out.push_back(Upgrade{tile});
                    }
                }
            }
            if(state_.ownership.holdings(tile).shares[idx(actor)] >= 1)
            {
                out.push_back(SellShares{tile, 1});
            }
        }

        if(manage->cardsPlayed < 1)
        {
            for(CardKind kind: state_.hands[idx(actor)].cards())
            {
                if(auto target = dummyTargetFor(state_, actor, kind))
                {
                    out.push_back(PlayCard{kind, *target});
                }
            }
        }

        const NodeId pos = state_.position[idx(actor)];
        if(std::holds_alternative<Exchange>(state_.board.node(pos).kind) &&
           !(state_.cash[idx(actor)] < Money{100}) && state_.hands[idx(actor)].cards().size() < Hand::kMax)
        {
            out.push_back(BuyCard{});
        }

        return out;
    }

    return out;
}

} // namespace sgl::syndicate
