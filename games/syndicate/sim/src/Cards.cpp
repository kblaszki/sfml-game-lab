#include <cstddef>
#include <cstdint>
#include <syndicate/sim/Cards.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr CardKind kAllKinds[] = {
    CardKind::HostileTakeover,
    CardKind::Audit,
    CardKind::MarketCrash,
    CardKind::ShortSqueeze,
    CardKind::Lobbying,
    CardKind::Embargo,
    CardKind::Subsidy,
    CardKind::Insider,
};

[[nodiscard]] bool targetMatches(CardKind kind, const CardTarget& target)
{
    switch(kind)
    {
    case CardKind::HostileTakeover:
        return std::holds_alternative<TakeoverTarget>(target);
    case CardKind::Audit:
        return std::holds_alternative<PlayerTarget>(target);
    case CardKind::MarketCrash:
    case CardKind::ShortSqueeze:
        return std::holds_alternative<SectorTarget>(target);
    case CardKind::Lobbying:
    case CardKind::Embargo:
        return std::holds_alternative<TileTarget>(target);
    case CardKind::Subsidy:
    case CardKind::Insider:
        return std::holds_alternative<NoTarget>(target);
    }
    return false;
}

} // namespace

std::expected<void, CardError> validateTarget(CardKind kind, const CardTarget& target)
{
    if(!targetMatches(kind, target))
    {
        return std::unexpected(CardError::BadTarget);
    }
    return {};
}

std::expected<std::vector<CardEffect>, CardError> describeEffect(CardKind kind, const CardTarget& target)
{
    if(const auto ok = validateTarget(kind, target); !ok)
    {
        return std::unexpected(ok.error());
    }

    std::vector<CardEffect> effects;
    switch(kind)
    {
    case CardKind::HostileTakeover:
    {
        const auto& t = std::get<TakeoverTarget>(target);
        // Price placeholder: Match fills via Economy (150% share price).
        effects.push_back(ShareGrab{t.victim, t.tile, Money{0}});
        break;
    }
    case CardKind::Audit:
    {
        const auto& t = std::get<PlayerTarget>(target);
        // Amount placeholder: Match computes 10% of positive cash.
        effects.push_back(CashDelta{t.player, Money{0}});
        break;
    }
    case CardKind::MarketCrash:
    {
        const auto& t = std::get<SectorTarget>(target);
        effects.push_back(MarketDelta{t.sector, -30});
        break;
    }
    case CardKind::ShortSqueeze:
    {
        const auto& t = std::get<SectorTarget>(target);
        // Market +30 only here; Match adds CashDelta of 50 per actor share in sector.
        effects.push_back(MarketDelta{t.sector, +30});
        break;
    }
    case CardKind::Lobbying:
    {
        const auto& t = std::get<TileTarget>(target);
        effects.push_back(FreeUpgrade{t.tile});
        break;
    }
    case CardKind::Embargo:
    {
        const auto& t = std::get<TileTarget>(target);
        effects.push_back(EmbargoSet{t.tile});
        break;
    }
    case CardKind::Subsidy:
        // PlayerId{} placeholder: Match substitutes the acting player.
        effects.push_back(CashDelta{PlayerId{}, Money{200}});
        break;
    case CardKind::Insider:
        effects.push_back(RevealNext{});
        break;
    }
    return effects;
}

void Deck::shuffleDraw(sgl::Pcg32& rng)
{
    if(draw_.empty())
    {
        return;
    }
    for(std::size_t i = draw_.size() - 1; i > 0; --i)
    {
        const auto j = static_cast<std::size_t>(rng.uniformBelow(static_cast<std::uint32_t>(i + 1)));
        std::swap(draw_[i], draw_[j]);
    }
}

void Deck::fillAndShuffle(sgl::Pcg32& rng)
{
    draw_.clear();
    discard_.clear();
    draw_.reserve(16);
    for(const CardKind kind: kAllKinds)
    {
        draw_.push_back(kind);
        draw_.push_back(kind);
    }
    shuffleDraw(rng);
}

std::expected<CardKind, CardError> Deck::draw(sgl::Pcg32& rng)
{
    if(draw_.empty())
    {
        if(discard_.empty())
        {
            return std::unexpected(CardError::EmptyDeck);
        }
        draw_ = std::move(discard_);
        discard_.clear();
        shuffleDraw(rng);
    }
    const CardKind kind = draw_.back();
    draw_.pop_back();
    return kind;
}

void Deck::discard(CardKind kind)
{
    discard_.push_back(kind);
}

std::size_t Deck::drawCount() const noexcept
{
    return draw_.size();
}

std::size_t Deck::discardCount() const noexcept
{
    return discard_.size();
}

std::expected<void, CardError> Hand::add(CardKind kind)
{
    if(cards_.size() >= kMax)
    {
        return std::unexpected(CardError::HandFull);
    }
    cards_.push_back(kind);
    return {};
}

bool Hand::remove(CardKind kind)
{
    for(auto it = cards_.begin(); it != cards_.end(); ++it)
    {
        if(*it == kind)
        {
            cards_.erase(it);
            return true;
        }
    }
    return false;
}

std::span<const CardKind> Hand::cards() const noexcept
{
    return cards_;
}

} // namespace sgl::syndicate
