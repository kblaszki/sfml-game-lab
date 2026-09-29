#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{

struct ShareLot
{
    NodeId tile;
    std::uint8_t count;
};

struct TradeOffer
{
    PlayerId from;
    PlayerId to;
    Money cashFrom; // cash the proposer pays the recipient
    Money cashTo;   // cash the recipient pays the proposer
    std::vector<ShareLot> sharesFrom; // shares proposer gives
    std::vector<ShareLot> sharesTo;
};

enum class TradeError
{
    SamePlayer,
    InsufficientCash,
    InsufficientShares,
    Empty,
};

template <class SharesOf>
[[nodiscard]] std::expected<void, TradeError> validateTrade(const TradeOffer& offer,
                                                            std::span<const Money> cash,
                                                            SharesOf sharesOf)
{
    if(offer.from == offer.to)
    {
        return std::unexpected(TradeError::SamePlayer);
    }

    const bool noCash = offer.cashFrom.v == 0 && offer.cashTo.v == 0;
    if(noCash && offer.sharesFrom.empty() && offer.sharesTo.empty())
    {
        return std::unexpected(TradeError::Empty);
    }

    if(cash.size() < kMaxPlayers)
    {
        return std::unexpected(TradeError::InsufficientCash);
    }

    const auto fromIdx = static_cast<std::size_t>(std::to_underlying(offer.from));
    const auto toIdx = static_cast<std::size_t>(std::to_underlying(offer.to));
    if(fromIdx >= kMaxPlayers || toIdx >= kMaxPlayers)
    {
        return std::unexpected(TradeError::InsufficientCash);
    }
    if(cash[fromIdx] < offer.cashFrom || cash[toIdx] < offer.cashTo)
    {
        return std::unexpected(TradeError::InsufficientCash);
    }

    auto enough = [&](PlayerId who, const std::vector<ShareLot>& lots) -> bool {
        for(std::size_t i = 0; i < lots.size(); ++i)
        {
            unsigned total = 0;
            for(const ShareLot& lot : lots)
            {
                if(lot.tile == lots[i].tile)
                {
                    total += lot.count;
                }
            }
            if(sharesOf(who, lots[i].tile) < total)
            {
                return false;
            }
        }
        return true;
    };

    if(!enough(offer.from, offer.sharesFrom) || !enough(offer.to, offer.sharesTo))
    {
        return std::unexpected(TradeError::InsufficientShares);
    }

    return {};
}

} // namespace sgl::syndicate
