#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <tower/app/Bindings.hpp>
#include <tower/app/GameOverScene.hpp>
#include <tower/app/MainMenuScene.hpp>
#include <tower/app/PauseScene.hpp>
#include <tower/app/PlayScene.hpp>
#include <tower/app/Scenes.hpp>

namespace
{

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::tower::bindTower(map)}
        , font{}
        , services{actions, font, 1}
        , stack{sgl::tower::pause(services)}
    {
    }

    sgl::ActionMap map;
    sgl::tower::TowerActions actions;
    sgl::FontId font;
    sgl::tower::AppServices services;
    sgl::SceneStack stack;
};

sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

} // namespace

TEST(ScenesTest, menuConfirmReplacesWithPlay)
{
    Fixture f;
    f.stack.push(sgl::tower::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    auto* play = dynamic_cast<sgl::tower::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, menuCancelQuits)
{
    Fixture f;
    f.stack.push(sgl::tower::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}

TEST(ScenesTest, pauseThenResume)
{
    Fixture f;
    f.stack.push(sgl::tower::play(f.services)());

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 2u);
    auto* pause = dynamic_cast<sgl::tower::PauseScene*>(f.stack.top());
    ASSERT_NE(pause, nullptr);
    EXPECT_FALSE(pause->traits().opaque);
    EXPECT_FALSE(pause->traits().pausable);

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 1u);
    auto* play = dynamic_cast<sgl::tower::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
}
