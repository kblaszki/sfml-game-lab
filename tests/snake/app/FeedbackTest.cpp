#include <mocks/AudioSpy.hpp>

#include <gtest/gtest.h>
#include <snake/app/Feedback.hpp>
#include <span>

namespace
{

sgl::snake::SoundIds makeIds()
{
    return sgl::snake::SoundIds{
        .eat = sgl::SoundId{.id = 1},
        .die = sgl::SoundId{.id = 2},
    };
}

} // namespace

TEST(FeedbackTest, atePlaysEat)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::snake::Feedback feedback{audio, ids};

    const sgl::snake::SnakeEvent event = sgl::snake::Ate{.score = 1};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.eat);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, diedPlaysDie)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::snake::Feedback feedback{audio, ids};

    const sgl::snake::SnakeEvent event = sgl::snake::Died{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.die);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, movedIsSilent)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::snake::Feedback feedback{audio, ids};

    const sgl::snake::SnakeEvent event = sgl::snake::Moved{};
    feedback.onEvents(std::span{&event, 1});

    EXPECT_TRUE(audio.plays.empty());
}

TEST(FeedbackTest, eatAndDiePlayInOrder)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::snake::Feedback feedback{audio, ids};

    const sgl::snake::SnakeEvent events[] = {
        sgl::snake::Moved{},
        sgl::snake::Ate{.score = 3},
        sgl::snake::Died{},
    };
    feedback.onEvents(events);

    ASSERT_EQ(audio.plays.size(), 2u);
    EXPECT_EQ(audio.plays[0].id, ids.eat);
    EXPECT_EQ(audio.plays[1].id, ids.die);
}
