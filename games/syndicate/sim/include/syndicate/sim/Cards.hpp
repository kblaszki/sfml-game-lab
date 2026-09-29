#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <sgl/core/Random.hpp>
#include <span>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>
#include <vector>

namespace sgl::syndicate
{

enum class CardKind : std::uint8_t
{
    HostileTakeover,
    Audit,
    MarketCrash,
    ShortSqueeze,
    Lobbying,
    Embargo,
    Subsidy,
    Insider,
};

struct NoTarget
{
};

struct SectorTarget
{
    Sector sector;
};

struct PlayerTarget
{
    PlayerId player;
};

struct TileTarget
{
    NodeId tile;
};

struct TakeoverTarget
{
    PlayerId victim;
    NodeId tile;
};

using CardTarget = std::variant<NoTarget, SectorTarget, PlayerTarget, TileTarget, TakeoverTarget>;

enum class CardError
{
    HandFull,
    EmptyDeck,
    BadTarget,
    Unknown,
};

struct MarketDelta
{
    Sector sector;
    int delta;
};

struct CashDelta
{
    PlayerId player;
    Money amount; // may be negative (pay bank)
};

struct ShareGrab
{
    PlayerId from;
    NodeId tile;
    Money price; // 1 share
};

struct FreeUpgrade
{
    NodeId tile;
};

struct EmbargoSet
{
    NodeId tile; // duration applied by Match later
};

struct RevealNext
{
};

using CardEffect = std::variant<MarketDelta, CashDelta, ShareGrab, FreeUpgrade, EmbargoSet, RevealNext>;

[[nodiscard]] std::expected<std::vector<CardEffect>, CardError> describeEffect(CardKind kind,
                                                                               const CardTarget& target);

[[nodiscard]] std::expected<void, CardError> validateTarget(CardKind kind, const CardTarget& target);

class Deck
{
public:
    void fillAndShuffle(sgl::Pcg32& rng);
    [[nodiscard]] std::expected<CardKind, CardError> draw(sgl::Pcg32& rng);
    void discard(CardKind kind);
    [[nodiscard]] std::size_t drawCount() const noexcept;
    [[nodiscard]] std::size_t discardCount() const noexcept;

private:
    void shuffleDraw(sgl::Pcg32& rng);

    std::vector<CardKind> draw_;
    std::vector<CardKind> discard_;
};

class Hand
{
public:
    static constexpr std::size_t kMax = 5;

    [[nodiscard]] std::expected<void, CardError> add(CardKind kind);
    [[nodiscard]] bool remove(CardKind kind);
    [[nodiscard]] std::span<const CardKind> cards() const noexcept;

private:
    std::vector<CardKind> cards_;
};

} // namespace sgl::syndicate
