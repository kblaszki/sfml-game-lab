#include <mocks/AudioSpy.hpp>

#include <cmath>
#include <gtest/gtest.h>
#include <sgl/core/Random.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <set>
#include <span>
#include <tetris/app/BoardRender.hpp>
#include <tetris/app/Feedback.hpp>
#include <tetris/sim/Events.hpp>
#include <tetris/sim/Grid.hpp>
#include <variant>

namespace
{

sgl::tetris::SoundIds makeIds()
{
    return sgl::tetris::SoundIds{
        .move = sgl::SoundId{.id = 1},
        .rotate = sgl::SoundId{.id = 2},
        .lock = sgl::SoundId{.id = 3},
        .line1 = sgl::SoundId{.id = 4},
        .line2 = sgl::SoundId{.id = 5},
        .line3 = sgl::SoundId{.id = 6},
        .tetris = sgl::SoundId{.id = 7},
        .levelUp = sgl::SoundId{.id = 8},
        .gameOver = sgl::SoundId{.id = 9},
    };
}

[[nodiscard]] float expectedBurstCenterY(std::uint8_t row)
{
    const sgl::tetris::Layout layout = sgl::tetris::defaultLayout();
    return layout.wellOrigin.y
           + layout.cell
                 * (static_cast<float>(sgl::tetris::Grid::kVisibleHeight - 1 - static_cast<int>(row)) + 0.5f);
}

} // namespace

TEST(FeedbackTest, lineClearPlaysOneSoundAndEmitsPerRow)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tetris::Feedback feedback{audio, ids, sgl::Pcg32{1}};

    const sgl::tetris::TetrisEvent event = sgl::tetris::LinesCleared{.rows = {3, 7}, .count = 2};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.line2);
    EXPECT_EQ(feedback.particleCount(), 2u * sgl::tetris::Feedback::kParticlesPerRow);
}

TEST(FeedbackTest, tetrisUsesTetrisSound)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tetris::Feedback feedback{audio, ids, sgl::Pcg32{2}};

    const sgl::tetris::TetrisEvent event = sgl::tetris::LinesCleared{.rows = {0, 1, 2, 3}, .count = 4};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.tetris);
    EXPECT_EQ(feedback.particleCount(), 4u * sgl::tetris::Feedback::kParticlesPerRow);
}

TEST(FeedbackTest, shiftPlaysMove)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tetris::Feedback feedback{audio, ids, sgl::Pcg32{3}};

    const sgl::tetris::TetrisEvent event = sgl::tetris::PieceShifted{};
    feedback.onEvents(std::span{&event, 1});

    ASSERT_EQ(audio.plays.size(), 1u);
    EXPECT_EQ(audio.plays[0].id, ids.move);
    EXPECT_EQ(feedback.particleCount(), 0u);
}

TEST(FeedbackTest, particlesExpire)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tetris::Feedback feedback{audio, ids, sgl::Pcg32{4}};

    const sgl::tetris::TetrisEvent event = sgl::tetris::LinesCleared{.rows = {0}, .count = 1};
    feedback.onEvents(std::span{&event, 1});
    ASSERT_GT(feedback.particleCount(), 0u);

    feedback.update(sgl::Seconds{1.f});
    EXPECT_EQ(feedback.particleCount(), 0u);
}

TEST(FeedbackTest, clearedRowIndicesDriveBurstOrigins)
{
    sgl::AudioSpy audio;
    const auto ids = makeIds();
    sgl::tetris::Feedback feedback{audio, ids, sgl::Pcg32{5}};

    const sgl::tetris::TetrisEvent event = sgl::tetris::LinesCleared{.rows = {5, 19}, .count = 2};
    feedback.onEvents(std::span{&event, 1});
    ASSERT_EQ(feedback.particleCount(), 2u * sgl::tetris::Feedback::kParticlesPerRow);

    sgl::RenderQueue queue;
    feedback.render(queue);

    std::set<int> centerYs;
    for(const sgl::Entry& entry: queue.sorted())
    {
        const auto* rect = std::get_if<sgl::RectCmd>(&entry.command);
        ASSERT_NE(rect, nullptr);
        const float centerY = rect->rect.pos.y + rect->rect.size.y * 0.5f;
        centerYs.insert(static_cast<int>(std::lround(centerY)));
    }

    EXPECT_TRUE(centerYs.contains(static_cast<int>(std::lround(expectedBurstCenterY(5)))));
    EXPECT_TRUE(centerYs.contains(static_cast<int>(std::lround(expectedBurstCenterY(19)))));
    EXPECT_FALSE(centerYs.contains(static_cast<int>(std::lround(expectedBurstCenterY(0)))));
}
