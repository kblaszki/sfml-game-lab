#include <mocks/AudioSpy.hpp>

#include <dodger/app/Feedback.hpp>
#include <gtest/gtest.h>
#include <span>

namespace
{

sgl::dodger::SoundIds makeIds()
{
    return sgl::dodger::SoundIds{
        .die = sgl::SoundId{.id = 1},
    };
}

} // namespace

TEST(FeedbackTest, diedPlaysDie)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::dodger::Feedback feedback{audio, ids};

    const sgl::dodger::DodgerEvent event = sgl::dodger::Died{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.die);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, hazardClearedIsSilent)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::dodger::Feedback feedback{audio, ids};

    const sgl::dodger::DodgerEvent event = sgl::dodger::HazardCleared{};
    feedback.onEvents(std::span{&event, 1});

    EXPECT_TRUE(audio.plays.empty());
}

TEST(FeedbackTest, clearThenDiePlaysOnlyDie)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::dodger::Feedback feedback{audio, ids};

    const sgl::dodger::DodgerEvent events[] = {
        sgl::dodger::HazardCleared{},
        sgl::dodger::Died{},
    };
    feedback.onEvents(events);

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.die);
}
