#include <sgl/core/Overloaded.hpp>
#include <syndicate/app/Feedback.hpp>
#include <variant>

namespace sgl::syndicate
{

Feedback::Feedback(sgl::AudioI& audio, SoundIds sounds)
    : audio_{audio}
    , sounds_{sounds}
{
}

void Feedback::onEvents(std::span<const GameEvent> events)
{
    for(const GameEvent& event: events)
    {
        std::visit(sgl::Overloaded{
                       [this](const Rolled&) { audio_.play(sounds_.roll, 1.f); },
                       [this](const FeePaid&) { audio_.play(sounds_.fee, 1.f); },
                       [this](const AuctionDone&) { audio_.play(sounds_.auction, 1.f); },
                       [this](const CardPlayed&) { audio_.play(sounds_.card, 1.f); },
                       [this](const MarketShifted&) { audio_.play(sounds_.market, 1.f); },
                       [this](const TradeResolved&) { audio_.play(sounds_.auction, 0.85f); },
                       [this](const SharesBought&) { audio_.play(sounds_.card, 0.7f); },
                       [this](const PaydayPaid&) { audio_.play(sounds_.market, 0.8f); },
                       [this](const Liquidated&) { audio_.play(sounds_.fee, 0.9f); },
                       [](const Moved&) {},
                       [](const GameEnded&) {},
                   },
                   event);
    }
}

} // namespace sgl::syndicate
