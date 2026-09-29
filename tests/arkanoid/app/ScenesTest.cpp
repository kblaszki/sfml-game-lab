#include <arkanoid/app/Bindings.hpp>
#include <arkanoid/app/GameplayScene.hpp>
#include <arkanoid/app/MainMenuScene.hpp>
#include <arkanoid/app/ResultScene.hpp>
#include <arkanoid/app/Scenes.hpp>
#include <arkanoid/sim/State.hpp>
#include <arkanoid/sim/Tuning.hpp>
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <sgl/audio/NullAudio.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/input/MouseButton.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <string>
#include <system_error>

namespace
{

[[nodiscard]] std::filesystem::path uniqueTempPath(std::string_view suffix)
{
    const auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    return std::filesystem::temp_directory_path() / ("arkanoid_app_" + stamp + std::string{suffix});
}

struct Fixture
{
    sgl::arkanoid::Actions actions = sgl::arkanoid::makeActions();
    sgl::arkanoid::TextureIds textures{};
    sgl::FontId font{};
    sgl::NullAudio audio{};
    sgl::arkanoid::SoundIds sounds{};
    std::filesystem::path scoresPath = uniqueTempPath(".scores");
    sgl::HighScoreTable highScores{sgl::arkanoid::kHighScoreCapacity};
    sgl::arkanoid::AppServices services{actions, textures, font, audio, sounds, highScores, scoresPath};
    sgl::ActionMap map = sgl::arkanoid::defaultBindings(actions);
    sgl::SceneStack stack{sgl::arkanoid::pause(services)};

    ~Fixture()
    {
        std::error_code ec;
        std::filesystem::remove(scoresPath, ec);
        std::filesystem::remove(std::filesystem::path(scoresPath.native() + std::filesystem::path(".tmp").native()),
                                ec);
    }
};

sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

sgl::InputState pointerAt(sgl::Vec2f pos, const sgl::ActionMap& map, bool pressed)
{
    sgl::InputState input;
    if(pressed)
    {
        input.apply(sgl::MouseDown{.button = sgl::MouseButton::Left, .pos = pos}, map);
    }
    else
    {
        input.apply(sgl::MouseMove{.pos = pos}, map);
    }
    return input;
}

sgl::InputState pointerUpAt(sgl::Vec2f pos, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::MouseUp{.button = sgl::MouseButton::Left, .pos = pos}, map);
    return input;
}

constexpr sgl::Vec2f kStartButtonCenter{640.f, 356.f};

void clearAllBricks(sgl::arkanoid::State& state)
{
    for(auto& brick: state.bricks)
    {
        brick.alive = false;
    }
    state.cleared = false;
}

} // namespace

TEST(ScenesShould, enterOnMainMenuStartsStage1)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_EQ(play->state().stage, sgl::arkanoid::StageId::Stage1);
}

TEST(ScenesShould, backspaceOnMainMenuQuits)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}

TEST(ScenesShould, clearingStage1AdvancesToStage2)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage1)());

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);

    const std::uint32_t score = play->state().score;
    clearAllBricks(play->state());

    f.stack.update(sgl::InputState{}, sgl::kTick);

    EXPECT_EQ(play->state().stage, sgl::arkanoid::StageId::Stage2);
    EXPECT_EQ(play->state().score, score);
}

TEST(ScenesShould, clearingStage3ShowsYouWinBanner)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage3)());

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);

    clearAllBricks(play->state());

    f.stack.update(sgl::InputState{}, sgl::kTick);

    EXPECT_EQ(play->state().stage, sgl::arkanoid::StageId::Stage3);
    ASSERT_TRUE(play->hud().banner.has_value());
    EXPECT_EQ(*play->hud().banner, "You win");
}

TEST(ScenesShould, escapeWhilePlayingPushesPause)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage1)());

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 2u);
}

TEST(ScenesTest, gameOverPushesResultLost)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage1)());

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    play->state().lives = 0;
    play->state().balls = {sgl::arkanoid::Ball{
        .pos = {100.f, sgl::arkanoid::designHeight + 1.f},
        .mode = sgl::arkanoid::BallMode::Live,
    }};

    f.stack.update(sgl::InputState{}, sgl::kTick);

    auto* result = dynamic_cast<sgl::arkanoid::ResultScene*>(f.stack.top());
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->outcome(), sgl::arkanoid::Outcome::Lost);
    EXPECT_EQ(f.stack.size(), 2u);
}

TEST(ScenesTest, stage3ClearPushesResultWon)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage3)());

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    clearAllBricks(play->state());

    f.stack.update(sgl::InputState{}, sgl::kTick);

    auto* result = dynamic_cast<sgl::arkanoid::ResultScene*>(f.stack.top());
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->outcome(), sgl::arkanoid::Outcome::Won);
    EXPECT_EQ(f.stack.size(), 2u);
}

TEST(ScenesTest, retryResetsScoreAndLives)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage3)());

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    play->state().score = 500;
    play->state().lives = 1;
    clearAllBricks(play->state());

    f.stack.update(sgl::InputState{}, sgl::kTick);
    ASSERT_NE(dynamic_cast<sgl::arkanoid::ResultScene*>(f.stack.top()), nullptr);

    f.stack.update(inputWith(sgl::Key::Enter, f.map), sgl::kTick);

    auto* fresh = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(fresh, nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_EQ(fresh->state().stage, sgl::arkanoid::StageId::Stage1);
    EXPECT_EQ(fresh->state().score, 0u);
    EXPECT_EQ(fresh->state().lives, sgl::arkanoid::startingLives);
}

TEST(ScenesTest, resultMenuReturnsToMenu)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage3)());

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    clearAllBricks(play->state());

    f.stack.update(sgl::InputState{}, sgl::kTick);
    ASSERT_NE(dynamic_cast<sgl::arkanoid::ResultScene*>(f.stack.top()), nullptr);

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    ASSERT_NE(dynamic_cast<sgl::arkanoid::MainMenuScene*>(f.stack.top()), nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, hoverWithoutClickDoesNothing)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::mainMenu(f.services)());

    f.stack.update(pointerAt(kStartButtonCenter, f.map, false), sgl::kTick);

    ASSERT_NE(dynamic_cast<sgl::arkanoid::MainMenuScene*>(f.stack.top()), nullptr);
    EXPECT_FALSE(f.stack.quitRequested());
}

TEST(ScenesTest, clickStarts)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::mainMenu(f.services)());

    // MenuModel activates on press+release inside the same button.
    f.stack.update(pointerAt(kStartButtonCenter, f.map, true), sgl::kTick);
    f.stack.update(pointerUpAt(kStartButtonCenter, f.map), sgl::kTick);

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_EQ(play->state().stage, sgl::arkanoid::StageId::Stage1);
}

TEST(ScenesTest, pauseResumeKeepsState)
{
    Fixture f;
    f.stack.push(sgl::arkanoid::gameplay(f.services, sgl::arkanoid::StageId::Stage1)());

    auto* play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    play->state().score = 42;
    play->state().lives = 2;
    const float paddleX = play->state().paddle.x;

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    EXPECT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    EXPECT_EQ(f.stack.size(), 1u);

    play = dynamic_cast<sgl::arkanoid::GameplayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_EQ(play->state().score, 42u);
    EXPECT_EQ(play->state().lives, 2u);
    EXPECT_FLOAT_EQ(play->state().paddle.x, paddleX);
}
