#include <mocks/AudioSpy.hpp>

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
#include <snake/app/Bindings.hpp>
#include <snake/app/GameOverScene.hpp>
#include <snake/app/MainMenuScene.hpp>
#include <snake/app/PauseScene.hpp>
#include <snake/app/PlayScene.hpp>
#include <snake/app/Scenes.hpp>
#include <snake/app/Sounds.hpp>
#include <string>
#include <string_view>
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
           / ("snake_scenes_" + std::to_string(stamp) + "_" + std::to_string(++seq) + ".scores");
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

TEST(ScenesTest, menuCancelQuits)
{
    Fixture f;
    f.stack.push(sgl::snake::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}

TEST(ScenesTest, playFactoryPushesPlayableScene)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    auto* play = dynamic_cast<sgl::snake::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, escapeRequestsPause)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 2u);
    auto* pause = dynamic_cast<sgl::snake::PauseScene*>(f.stack.top());
    EXPECT_NE(pause, nullptr);
}

TEST(ScenesTest, pauseCancelResumes)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());
    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    ASSERT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_NE(dynamic_cast<sgl::snake::PlayScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, gameOverRecordsHighScore)
{
    Fixture f;
    constexpr int score = 2500;

    f.stack.push(sgl::snake::gameOver(f.services, score)());

    auto* over = dynamic_cast<sgl::snake::GameOverScene*>(f.stack.top());
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
    const auto parsed = sgl::parse(*text, sgl::snake::kHighScoreCapacity);
    ASSERT_TRUE(parsed.has_value()) << sgl::describe(parsed.error());
    ASSERT_FALSE(parsed->entries().empty());
    EXPECT_EQ(parsed->entries().front(), static_cast<std::uint32_t>(score));
}

TEST(ScenesTest, gameOverMenuReturnsToMainMenu)
{
    Fixture f;
    f.stack.push(sgl::snake::gameOver(f.services, 10)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    EXPECT_NE(dynamic_cast<sgl::snake::MainMenuScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, lowScoreDoesNotEnterFullTable)
{
    Fixture f;
    for(const std::uint32_t existing: {100u, 80u, 60u, 40u, 20u})
    {
        ASSERT_TRUE(f.highScores.insert(existing).has_value());
    }

    f.stack.push(sgl::snake::gameOver(f.services, 10)());

    auto* over = dynamic_cast<sgl::snake::GameOverScene*>(f.stack.top());
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
    const sgl::snake::SnakeActions actions = sgl::snake::bindSnake(map);
    sgl::AudioSpy audio;
    const sgl::snake::SoundIds sounds = testSoundIds();
    const std::filesystem::path blocker = uniqueScoresPath();
    {
        std::ofstream out{blocker};
        out << "not-a-directory";
    }
    const std::filesystem::path scoresPath = blocker / "snake.scores";
    sgl::HighScoreTable highScores{sgl::snake::kHighScoreCapacity};
    const sgl::snake::AppServices services{actions, sgl::FontId{}, 1, audio, sounds, scoresPath, highScores};
    sgl::SceneStack stack{sgl::snake::pause(services)};
    stack.push(sgl::snake::gameOver(services, 40)());

    auto* over = dynamic_cast<sgl::snake::GameOverScene*>(stack.top());
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
    EXPECT_NE(dynamic_cast<sgl::snake::MainMenuScene*>(stack.top()), nullptr);

    std::error_code ec;
    std::filesystem::remove(blocker, ec);
}
