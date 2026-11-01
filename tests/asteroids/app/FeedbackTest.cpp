#include <mocks/AudioSpy.hpp>

#include <asteroids/app/Feedback.hpp>
#include <gtest/gtest.h>
#include <span>

namespace
{

sgl::asteroids::SoundIds makeIds()
{
    return sgl::asteroids::SoundIds{
        .fire = sgl::SoundId{.id = 1},
        .hit = sgl::SoundId{.id = 2},
        .die = sgl::SoundId{.id = 3},
    };
}

} // namespace

TEST(FeedbackTest, shotFiredPlaysFire)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::asteroids::Feedback feedback{audio, ids};

    const sgl::asteroids::AsteroidsEvent event = sgl::asteroids::ShotFired{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.fire);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, rockHitPlaysHit)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::asteroids::Feedback feedback{audio, ids};

    const sgl::asteroids::AsteroidsEvent event = sgl::asteroids::RockHit{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.hit);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, diedPlaysDie)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::asteroids::Feedback feedback{audio, ids};

    const sgl::asteroids::AsteroidsEvent event = sgl::asteroids::Died{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.die);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, waveClearedIsSilent)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::asteroids::Feedback feedback{audio, ids};

    const sgl::asteroids::AsteroidsEvent event = sgl::asteroids::WaveCleared{};
    feedback.onEvents(std::span{&event, 1});

    EXPECT_TRUE(audio.plays.empty());
}

TEST(FeedbackTest, fireHitAndDiePlayInOrder)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::asteroids::Feedback feedback{audio, ids};

    const sgl::asteroids::AsteroidsEvent events[] = {
        sgl::asteroids::WaveCleared{},
        sgl::asteroids::ShotFired{},
        sgl::asteroids::RockHit{.score = 20},
        sgl::asteroids::Died{},
    };
    feedback.onEvents(events);

    ASSERT_EQ(audio.plays.size(), 3u);
    EXPECT_EQ(audio.plays[0].id, ids.fire);
    EXPECT_EQ(audio.plays[1].id, ids.hit);
    EXPECT_EQ(audio.plays[2].id, ids.die);
}
