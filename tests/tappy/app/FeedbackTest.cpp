#include <mocks/AudioSpy.hpp>

#include <gtest/gtest.h>
#include <span>
#include <tappy/app/Feedback.hpp>

namespace
{

sgl::tappy::SoundIds makeIds()
{
    return sgl::tappy::SoundIds{
        .flap = sgl::SoundId{.id = 1},
        .clear = sgl::SoundId{.id = 2},
        .die = sgl::SoundId{.id = 3},
    };
}

} // namespace

TEST(FeedbackTest, flapClearDiePlay)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tappy::Feedback feedback{audio, ids};

    const sgl::tappy::TappyEvent events[] = {
        sgl::tappy::Flapped{},
        sgl::tappy::GateCleared{.gateIndex = 0},
        sgl::tappy::Died{},
    };
    feedback.onEvents(events);

    ASSERT_EQ(audio.plays.size(), 3u);
    EXPECT_EQ(audio.plays[0].id, ids.flap);
    EXPECT_EQ(audio.plays[1].id, ids.clear);
    EXPECT_EQ(audio.plays[2].id, ids.die);
}
