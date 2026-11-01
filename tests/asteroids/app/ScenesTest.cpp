#include <mocks/AudioSpy.hpp>

#include <asteroids/app/Bindings.hpp>
#include <asteroids/app/GameOverScene.hpp>
#include <asteroids/app/MainMenuScene.hpp>
#include <asteroids/app/PauseScene.hpp>
#include <asteroids/app/PlayScene.hpp>
#include <asteroids/app/Scenes.hpp>
#include <asteroids/app/Sounds.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/save/SaveFile.hpp>
#include <sgl/save/SaveFormat.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <string>
#include <string_view>
#include <system_error>

namespace
{

sgl::asteroids::SoundIds testSoundIds()
{
    return sgl::asteroids::SoundIds{
        .fire = sgl::SoundId{.id = 1},
        .hit = sgl::SoundId{.id = 2},
        .die = sgl::SoundId{.id = 3},
    };
}

[[nodiscard]] std::filesystem::path uniqueScoresPath()
{
    static int seq = 0;
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
           / ("asteroids_scenes_" + std::to_string(stamp) + "_" + std::to_string(++seq) + ".scores");
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
        , actions{sgl::asteroids::bindAsteroids(map)}
        , font{}
        , audio{}
        , sounds{testSoundIds()}
        , scoresPath{uniqueScoresPath()}
        , highScores{sgl::asteroids::kHighScoreCapacity}
        , services{actions, font, 1, audio, sounds, scoresPath, highScores}
        , stack{sgl::asteroids::pause(services)}
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
    sgl::asteroids::AsteroidsActions actions;
    sgl::FontId font;
    sgl::AudioSpy audio;
    sgl::asteroids::SoundIds sounds;
    std::filesystem::path scoresPath;
    sgl::HighScoreTable highScores;
    sgl::asteroids::AppServices services;
    sgl::SceneStack stack;
};

sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

} // namespace

TEST(ScenesTest, menuConfirmPushesPlay)
{
    Fixture f;
    f.stack.push(sgl::asteroids::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    auto* play = dynamic_cast<sgl::asteroids::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, menuCancelQuits)
{
    Fixture f;
    f.stack.push(sgl::asteroids::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}

TEST(ScenesTest, escapeRequestsPause)
{
    Fixture f;
    f.stack.push(sgl::asteroids::play(f.services)());

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 2u);
    auto* pause = dynamic_cast<sgl::asteroids::PauseScene*>(f.stack.top());
    ASSERT_NE(pause, nullptr);
    EXPECT_FALSE(pause->traits().opaque);
    EXPECT_FALSE(pause->traits().pausable);
}

TEST(ScenesTest, pauseCancelResumes)
{
    Fixture f;
    f.stack.push(sgl::asteroids::play(f.services)());
    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    ASSERT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_NE(dynamic_cast<sgl::asteroids::PlayScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, pauseMenuReturnsToMainMenu)
{
    Fixture f;
    f.stack.push(sgl::asteroids::play(f.services)());
    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    ASSERT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Right, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_NE(dynamic_cast<sgl::asteroids::MainMenuScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, gameOverMenuReturnsToMainMenu)
{
    Fixture f;
    f.stack.push(sgl::asteroids::gameOver(f.services, 40)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    EXPECT_NE(dynamic_cast<sgl::asteroids::MainMenuScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, gameOverRecordsHighScore)
{
    Fixture f;
    constexpr int score = 2500;

    f.stack.push(sgl::asteroids::gameOver(f.services, score)());

    auto* over = dynamic_cast<sgl::asteroids::GameOverScene*>(f.stack.top());
    ASSERT_NE(over, nullptr);
    EXPECT_TRUE(over->isNewRecord());
    EXPECT_FALSE(over->saveFailed());
    ASSERT_FALSE(f.highScores.entries().empty());
    EXPECT_EQ(f.highScores.entries().front(), static_cast<std::uint32_t>(score));

    sgl::RenderQueue queue;
    over->render(queue);
    EXPECT_TRUE(queueContainsText(queue, "#1 2500"));
    EXPECT_TRUE(queueContainsText(queue, "New record"));

    const auto text = sgl::readTextFile(f.scoresPath);
    ASSERT_TRUE(text.has_value()) << sgl::describe(text.error());
    const auto parsed = sgl::parse(*text, sgl::asteroids::kHighScoreCapacity);
    ASSERT_TRUE(parsed.has_value()) << sgl::describe(parsed.error());
    ASSERT_FALSE(parsed->entries().empty());
    EXPECT_EQ(parsed->entries().front(), static_cast<std::uint32_t>(score));
}

TEST(ScenesTest, lowScoreDoesNotEnterFullTable)
{
    Fixture f;
    for(const std::uint32_t existing: {100u, 80u, 60u, 40u, 20u})
    {
        ASSERT_TRUE(f.highScores.insert(existing).has_value());
    }

    f.stack.push(sgl::asteroids::gameOver(f.services, 10)());

    auto* over = dynamic_cast<sgl::asteroids::GameOverScene*>(f.stack.top());
    ASSERT_NE(over, nullptr);
    EXPECT_FALSE(over->isNewRecord());
    EXPECT_FALSE(over->saveFailed());
    EXPECT_EQ(f.highScores.entries().back(), 20u);
    EXPECT_FALSE(std::filesystem::exists(f.scoresPath));

    sgl::RenderQueue queue;
    over->render(queue);
    EXPECT_TRUE(queueContainsText(queue, "#1 100"));
    EXPECT_TRUE(queueContainsText(queue, "#5 20"));
    EXPECT_TRUE(queueContainsText(queue, "Score 10"));
    EXPECT_FALSE(queueContainsText(queue, "New record"));
}

TEST(ScenesTest, saveFailureStillShowsGameOver)
{
    sgl::ActionMap map;
    const sgl::asteroids::AsteroidsActions actions = sgl::asteroids::bindAsteroids(map);
    sgl::AudioSpy audio;
    const sgl::asteroids::SoundIds sounds = testSoundIds();
    const std::filesystem::path blocker = uniqueScoresPath();
    {
        std::ofstream out{blocker};
        out << "not-a-directory";
    }
    const std::filesystem::path scoresPath = blocker / "asteroids.scores";
    sgl::HighScoreTable highScores{sgl::asteroids::kHighScoreCapacity};
    const sgl::asteroids::AppServices services{actions, sgl::FontId{}, 1, audio, sounds, scoresPath, highScores};
    sgl::SceneStack stack{sgl::asteroids::pause(services)};
    stack.push(sgl::asteroids::gameOver(services, 40)());

    auto* over = dynamic_cast<sgl::asteroids::GameOverScene*>(stack.top());
    ASSERT_NE(over, nullptr);
    EXPECT_TRUE(over->isNewRecord());
    EXPECT_TRUE(over->saveFailed());
    EXPECT_EQ(over->score(), 40);

    sgl::RenderQueue queue;
    over->render(queue);
    EXPECT_TRUE(queueContainsText(queue, "Save failed"));
    EXPECT_TRUE(queueContainsText(queue, "#1 40"));
    EXPECT_TRUE(queueContainsText(queue, "New record"));

    stack.update(inputWith(sgl::Key::Enter, map), sgl::kTick);
    EXPECT_NE(dynamic_cast<sgl::asteroids::MainMenuScene*>(stack.top()), nullptr);

    std::error_code ec;
    std::filesystem::remove(blocker, ec);
}
