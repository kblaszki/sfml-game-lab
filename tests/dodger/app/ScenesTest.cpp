#include <dodger/app/Bindings.hpp>
#include <dodger/app/GameOverScene.hpp>
#include <dodger/app/MainMenuScene.hpp>
#include <dodger/app/PauseScene.hpp>
#include <dodger/app/PlayScene.hpp>
#include <dodger/app/Scenes.hpp>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/scene/SceneStack.hpp>

namespace
{

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::dodger::bindDodger(map)}
        , font{}
        , services{actions, font, 1}
        , stack{sgl::dodger::pause(services)}
    {
    }

    sgl::ActionMap map;
    sgl::dodger::DodgerActions actions;
    sgl::FontId font;
    sgl::dodger::AppServices services;
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
    f.stack.push(sgl::dodger::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    auto* play = dynamic_cast<sgl::dodger::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, menuCancelQuits)
{
    Fixture f;
    f.stack.push(sgl::dodger::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}

TEST(ScenesTest, escapeRequestsPause)
{
    Fixture f;
    f.stack.push(sgl::dodger::play(f.services)());

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 2u);
    auto* pause = dynamic_cast<sgl::dodger::PauseScene*>(f.stack.top());
    ASSERT_NE(pause, nullptr);
    EXPECT_FALSE(pause->traits().opaque);
    EXPECT_FALSE(pause->traits().pausable);
}

TEST(ScenesTest, pauseCancelResumes)
{
    Fixture f;
    f.stack.push(sgl::dodger::play(f.services)());
    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    ASSERT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_NE(dynamic_cast<sgl::dodger::PlayScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, pauseMenuReturnsToMainMenu)
{
    Fixture f;
    f.stack.push(sgl::dodger::play(f.services)());
    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    ASSERT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Right, f.map), sgl::kTick);
    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_NE(dynamic_cast<sgl::dodger::MainMenuScene*>(f.stack.top()), nullptr);
}

TEST(ScenesTest, gameOverMenuReturnsToMainMenu)
{
    Fixture f;
    f.stack.push(sgl::dodger::gameOver(f.services, 40)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    EXPECT_NE(dynamic_cast<sgl::dodger::MainMenuScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
}
