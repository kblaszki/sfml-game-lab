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
#include <string_view>
#include <system_error>
#include <tappy/app/Bindings.hpp>
#include <tappy/app/GameOverScene.hpp>
#include <tappy/app/MainMenuScene.hpp>
#include <tappy/app/PauseScene.hpp>
#include <tappy/app/PlayScene.hpp>
#include <tappy/app/Scenes.hpp>
#include <tappy/app/Sounds.hpp>

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
           / ("tappy_scenes_" + std::to_string(stamp) + "_" + std::to_string(++seq) + ".scores");
}

[[nodiscard]] bool queueContainsText(sgl::RenderQueue& queue, std::string_view needle)
{
    for(const sgl::Entry& entry: queue.sorted())
    {
        if(const auto* text = std::get_if<sgl::TextCmd>(&entry.command))
        {
            if(text->text.find(needle) != std::string::npos)
            {
                return true;
            }
        }
    }
    return false;
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

sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

} // namespace

TEST(ScenesTest, playFromMenu)
{
    Fixture f;
    f.stack.push(sgl::tappy::mainMenu(f.services)());
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);
    EXPECT_NE(dynamic_cast<sgl::tappy::PlayScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, pauseAndResume)
{
    Fixture f;
    f.stack.push(sgl::tappy::play(f.services)());
    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    EXPECT_NE(dynamic_cast<sgl::tappy::PauseScene*>(f.stack.top()), nullptr);

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);
    EXPECT_NE(dynamic_cast<sgl::tappy::PlayScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, gameOverShowsScore)
{
    Fixture f;
    f.stack.push(sgl::tappy::gameOver(f.services, 7)());
    auto* over = dynamic_cast<sgl::tappy::GameOverScene*>(f.stack.top());
    ASSERT_NE(over, nullptr);
    EXPECT_EQ(over->score(), 7);

    sgl::RenderQueue queue;
    over->render(queue);
    EXPECT_TRUE(queueContainsText(queue, "Score 7"));
}
