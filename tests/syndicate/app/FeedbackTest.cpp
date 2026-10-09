#include <mocks/AudioSpy.hpp>

#include <gtest/gtest.h>
#include <span>
#include <syndicate/app/Feedback.hpp>
#include <syndicate/sim/Events.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>

namespace
{

[[nodiscard]] sgl::syndicate::SoundIds makeIds()
{
    return sgl::syndicate::SoundIds{
        .roll = sgl::SoundId{.id = 1},
        .fee = sgl::SoundId{.id = 2},
        .auction = sgl::SoundId{.id = 3},
        .card = sgl::SoundId{.id = 4},
        .market = sgl::SoundId{.id = 5},
    };
}

} // namespace

TEST(FeedbackTest, rolledPlaysSound)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::syndicate::Feedback feedback{audio, ids};

    const sgl::syndicate::GameEvent event =
        sgl::syndicate::Rolled{.player = sgl::syndicate::PlayerId{0}, .roll = 4};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_GE(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.roll);
}

TEST(FeedbackTest, feePlaysSound)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::syndicate::Feedback feedback{audio, ids};

    const sgl::syndicate::GameEvent event = sgl::syndicate::FeePaid{
        .payer = sgl::syndicate::PlayerId{0},
        .tile = sgl::syndicate::NodeId{1},
        .total = sgl::syndicate::Money{50},
    };
    feedback.onEvents(std::span{&event, 1});

    ASSERT_GE(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.fee);
}

TEST(FeedbackTest, tradeAndPaydayPlaySounds)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::syndicate::Feedback feedback{audio, ids};

    const sgl::syndicate::GameEvent trade = sgl::syndicate::TradeResolved{
        .from = sgl::syndicate::PlayerId{0},
        .to = sgl::syndicate::PlayerId{1},
        .accepted = true,
    };
    feedback.onEvents(std::span{&trade, 1});
    ASSERT_GE(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays.back().id, ids.auction);

    const sgl::syndicate::GameEvent payday = sgl::syndicate::PaydayPaid{
        .player = sgl::syndicate::PlayerId{0},
        .salary = sgl::syndicate::Money{100},
        .dividends = sgl::syndicate::Money{20},
        .interest = sgl::syndicate::Money{0},
    };
    feedback.onEvents(std::span{&payday, 1});
    EXPECT_EQ(audio.plays.back().id, ids.market);
}
