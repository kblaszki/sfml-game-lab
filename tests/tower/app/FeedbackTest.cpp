#include <mocks/AudioSpy.hpp>

#include <gtest/gtest.h>
#include <span>
#include <tower/app/Feedback.hpp>

namespace
{

sgl::tower::SoundIds makeIds()
{
    return sgl::tower::SoundIds{
        .jump = sgl::SoundId{.id = 1},
        .land = sgl::SoundId{.id = 2},
        .die = sgl::SoundId{.id = 3},
        .wall = sgl::SoundId{.id = 4},
        .bonus = sgl::SoundId{.id = 5},
        .edge = sgl::SoundId{.id = 6},
    };
}

} // namespace

TEST(FeedbackTest, landedPlaysLand)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tower::Feedback feedback{audio, ids};

    const sgl::tower::TowerEvent event = sgl::tower::Landed{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.land);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, diedPlaysDie)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tower::Feedback feedback{audio, ids};

    const sgl::tower::TowerEvent event = sgl::tower::Died{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.die);
    EXPECT_FLOAT_EQ(audio.plays[0].volume, 1.f);
}

TEST(FeedbackTest, wallAndBonusPlay)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tower::Feedback feedback{audio, ids};

    const sgl::tower::TowerEvent events[] = {
        sgl::tower::WallBounced{},
        sgl::tower::BonusPicked{.kind = sgl::tower::BonusKind::Speed, .points = 0},
    };
    feedback.onEvents(events);

    ASSERT_EQ(audio.plays.size(), 2u);
    EXPECT_EQ(audio.plays[0].id, ids.wall);
    EXPECT_EQ(audio.plays[1].id, ids.bonus);
}

TEST(FeedbackTest, edgeJumpedPlaysEdge)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tower::Feedback feedback{audio, ids};

    const sgl::tower::TowerEvent event = sgl::tower::EdgeJumped{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.edge);
}

TEST(FeedbackTest, landAndDiePlayInOrder)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tower::Feedback feedback{audio, ids};

    const sgl::tower::TowerEvent events[] = {
        sgl::tower::Landed{},
        sgl::tower::Died{},
    };
    feedback.onEvents(events);

    ASSERT_EQ(audio.plays.size(), 2u);
    EXPECT_EQ(audio.plays[0].id, ids.land);
    EXPECT_EQ(audio.plays[1].id, ids.die);
}
