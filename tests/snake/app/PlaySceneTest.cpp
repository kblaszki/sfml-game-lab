#include <mocks/AudioSpy.hpp>

#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <snake/app/Bindings.hpp>
#include <snake/app/GameOverScene.hpp>
#include <snake/app/PlayScene.hpp>
#include <snake/app/Scenes.hpp>
#include <snake/app/Sounds.hpp>
#include <snake/sim/Types.hpp>
#include <string>
#include <system_error>

namespace
{

sgl::snake::SoundIds testSoundIds()
{
    return sgl::snake::SoundIds{
        .eat = sgl::SoundId{.id = 1},
        .die = sgl::SoundId{.id = 2},
    };
}

[[nodiscard]] std::filesystem::path uniqueScoresPath()
{
    static int seq = 0;
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
           / ("snake_play_" + std::to_string(stamp) + "_" + std::to_string(++seq) + ".scores");
}

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::snake::bindSnake(map)}
        , font{}
        , audio{}
        , sounds{testSoundIds()}
        , scoresPath{uniqueScoresPath()}
        , highScores{sgl::snake::kHighScoreCapacity}
        , services{actions, font, 1, audio, sounds, scoresPath, highScores}
        , stack{sgl::snake::pause(services)}
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
    sgl::snake::SnakeActions actions;
    sgl::FontId font;
    sgl::AudioSpy audio;
    sgl::snake::SoundIds sounds;
    std::filesystem::path scoresPath;
    sgl::HighScoreTable highScores;
    sgl::snake::AppServices services;
    sgl::SceneStack stack;
};

sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

} // namespace

TEST(PlaySceneTest, stepsSimWithInputState)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    auto* play = dynamic_cast<sgl::snake::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    ASSERT_FALSE(play->game().body().empty());
    const sgl::snake::Cell before = play->game().body().front();
    EXPECT_EQ(play->game().dir(), sgl::snake::Dir::Right);

    f.stack.update(inputWith(sgl::Key::Up, f.map), sgl::Seconds{0.15f});

    EXPECT_EQ(play->game().dir(), sgl::snake::Dir::Up);
    EXPECT_EQ(play->game().body().front(), (sgl::snake::Cell{before.x, before.y + 1}));
    EXPECT_FALSE(play->game().over());
}

TEST(PlaySceneTest, diesAfterRunningIntoWall)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    sgl::InputState input;
    sgl::snake::GameOverScene* over = nullptr;
    for(int i = 0; i < 300 && over == nullptr; ++i)
    {
        f.stack.update(input, sgl::kTick);
        over = dynamic_cast<sgl::snake::GameOverScene*>(f.stack.top());
    }

    ASSERT_NE(over, nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_GE(over->score(), 0);

    bool heardDie = false;
    for(const sgl::AudioSpy::Play& play: f.audio.plays)
    {
        if(play.id == f.sounds.die)
        {
            heardDie = true;
        }
    }
    EXPECT_TRUE(heardDie);
}
