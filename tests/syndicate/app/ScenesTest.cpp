#include <mocks/AudioSpy.hpp>

#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/ui/SettingsScene.hpp>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/HelpScene.hpp>
#include <syndicate/app/MainMenuScene.hpp>
#include <syndicate/app/PauseMenuScene.hpp>
#include <syndicate/app/PlayScene.hpp>
#include <syndicate/app/Scenes.hpp>
#include <syndicate/app/SetupScene.hpp>
#include <syndicate/app/Sounds.hpp>
#include <system_error>

namespace
{

struct Fixture
{
    Fixture()
        : scoresPath{std::filesystem::temp_directory_path()
                     / ("syndicate_scenes_"
                        + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".scores")}
        , highScores{sgl::syndicate::kHighScoreCapacity}
        , actions{sgl::syndicate::bindSyndicate(map)}
        , services{actions, font, 1, audio, sounds, scoresPath, highScores}
        , stack{sgl::syndicate::pause(services)}
    {
    }

    ~Fixture()
    {
        std::error_code ec;
        std::filesystem::remove(scoresPath, ec);
        std::filesystem::remove(std::filesystem::path(scoresPath.native() + std::filesystem::path(".tmp").native()),
                                ec);
    }

    sgl::ActionMap map{};
    sgl::FontId font{.id = 1};
    sgl::AudioSpy audio{};
    sgl::syndicate::SoundIds sounds{
        .roll = sgl::SoundId{.id = 1},
        .fee = sgl::SoundId{.id = 2},
        .auction = sgl::SoundId{.id = 3},
        .card = sgl::SoundId{.id = 4},
        .market = sgl::SoundId{.id = 5},
    };
    std::filesystem::path scoresPath;
    sgl::HighScoreTable highScores;
    sgl::syndicate::SyndicateActions actions;
    sgl::syndicate::SyndicateServices services;
    sgl::SceneStack stack;
};

[[nodiscard]] sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

} // namespace

TEST(ScenesTest, menuCancelRequestsQuitApp)
{
    Fixture f;
    f.stack.push(sgl::syndicate::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}

TEST(ScenesTest, menuConfirmPushesSetup)
{
    Fixture f;
    f.stack.push(sgl::syndicate::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    ASSERT_NE(dynamic_cast<sgl::syndicate::SetupScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 2u);
}

TEST(ScenesTest, menuSettingsPushesSettingsScene)
{
    Fixture f;
    f.stack.push(sgl::syndicate::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    ASSERT_NE(dynamic_cast<sgl::SettingsScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 2u);
}

TEST(ScenesTest, menuHelpPushesHelpScene)
{
    Fixture f;
    f.stack.push(sgl::syndicate::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    ASSERT_NE(dynamic_cast<sgl::syndicate::HelpScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 2u);
}

TEST(ScenesTest, setupConfirmPushesPlay)
{
    Fixture f;
    f.stack.push(sgl::syndicate::setup(f.services)());

    // Seats button focused first; Down to Confirm (index seatCount+1 = 3 for 2 seats).
    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    ASSERT_NE(dynamic_cast<sgl::syndicate::PlayScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, pauseResumePops)
{
    Fixture f;
    f.stack.push(std::make_unique<sgl::syndicate::MainMenuScene>(f.services));
    f.stack.push(sgl::syndicate::pause(f.services)());

    ASSERT_NE(dynamic_cast<sgl::syndicate::PauseMenuScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);
    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_FALSE(f.stack.quitRequested());
}

TEST(ScenesTest, pauseSettingsPushes)
{
    Fixture f;
    f.stack.push(std::make_unique<sgl::syndicate::MainMenuScene>(f.services));
    f.stack.push(sgl::syndicate::pause(f.services)());

    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    ASSERT_NE(dynamic_cast<sgl::SettingsScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 3u);
}

TEST(ScenesTest, pauseQuitRequestsQuitApp)
{
    Fixture f;
    f.stack.push(std::make_unique<sgl::syndicate::MainMenuScene>(f.services));
    f.stack.push(sgl::syndicate::pause(f.services)());

    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Down, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}
