#include <mocks/AudioSpy.hpp>

#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <system_error>
#include <variant>
#include <tappy/app/Bindings.hpp>
#include <tappy/app/PlayScene.hpp>
#include <tappy/app/Scenes.hpp>
#include <tappy/app/Sounds.hpp>
#include <tappy/sim/Types.hpp>

namespace
{

sgl::tappy::SoundIds testSoundIds()
{
    return sgl::tappy::SoundIds{
        .flap = sgl::SoundId{.id = 1},
        .clear = sgl::SoundId{.id = 2},
        .die = sgl::SoundId{.id = 3},
    };
}

[[nodiscard]] std::filesystem::path uniqueScoresPath()
{
    static int seq = 0;
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
           / ("tappy_play_" + std::to_string(stamp) + "_" + std::to_string(++seq) + ".scores");
}

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::tappy::bindTappy(map)}
        , font{}
        , audio{}
        , sounds{testSoundIds()}
        , scoresPath{uniqueScoresPath()}
        , highScores{sgl::tappy::kHighScoreCapacity}
        , services{actions, font, 1, audio, sounds, scoresPath, highScores}
        , stack{sgl::tappy::pause(services)}
    {
    }

    ~Fixture()
    {
        std::error_code ec;
        std::filesystem::remove(scoresPath, ec);
        std::filesystem::remove(std::filesystem::path(scoresPath.native() + std::filesystem::path(".tmp").native()),
                                ec);
    }

    sgl::ActionMap map;
    sgl::tappy::TappyActions actions;
    sgl::FontId font;
    sgl::AudioSpy audio;
    sgl::tappy::SoundIds sounds;
    std::filesystem::path scoresPath;
    sgl::HighScoreTable highScores;
    sgl::tappy::AppServices services;
    sgl::SceneStack stack;
};

[[nodiscard]] int countRects(sgl::RenderQueue& queue)
{
    int n = 0;
    for(const sgl::Entry& entry: queue.sorted())
    {
        if(std::holds_alternative<sgl::RectCmd>(entry.command))
        {
            ++n;
        }
    }
    return n;
}

} // namespace

TEST(PlaySceneTest, spaceFlapRaisesVyAndPlays)
{
    Fixture f;
    f.services.tuning.gravity = 0.f;
    f.stack.push(sgl::tappy::play(f.services)());
    auto* play = dynamic_cast<sgl::tappy::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);

    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = sgl::Key::Space}, f.map);
    f.stack.update(input, sgl::kTick);

    EXPECT_FLOAT_EQ(play->game().player().vel.y, f.services.tuning.flapSpeed);
    ASSERT_FALSE(f.audio.plays.empty());
    EXPECT_EQ(f.audio.plays.front().id, f.sounds.flap);
}

TEST(PlaySceneTest, drawsPlayerAndGates)
{
    Fixture f;
    f.stack.push(sgl::tappy::play(f.services)());
    auto* play = dynamic_cast<sgl::tappy::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    ASSERT_FALSE(play->game().gates().empty());

    sgl::RenderQueue queue;
    play->render(queue);
    EXPECT_GE(countRects(queue), 3);
}

TEST(PlaySceneTest, fallingDiesToGameOver)
{
    Fixture f;
    f.services.tuning.gravity = 8000.f;
    f.services.tuning.scrollBase = 0.f;
    f.services.tuning.gateSpacing = 10000.f;
    f.stack.push(sgl::tappy::play(f.services)());

    for(int i = 0; i < 120; ++i)
    {
        f.stack.update({}, sgl::kTick);
        if(dynamic_cast<sgl::tappy::PlayScene*>(f.stack.top()) == nullptr)
        {
            break;
        }
    }
    EXPECT_EQ(dynamic_cast<sgl::tappy::PlayScene*>(f.stack.top()), nullptr);
}
