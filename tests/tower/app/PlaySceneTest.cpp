#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <tower/app/Bindings.hpp>
#include <tower/app/GameOverScene.hpp>
#include <tower/app/PlayScene.hpp>
#include <tower/app/Scenes.hpp>
#include <tower/sim/Tuning.hpp>
#include <tower/sim/Types.hpp>
#include <variant>

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

[[nodiscard]] sgl::Rect<float> screenRect(sgl::tower::Vec2f pos, sgl::tower::Vec2f size, float cameraBottom)
{
    return sgl::Rect<float>{
        .pos = {pos.x, sgl::tower::kViewH - (pos.y + size.y - cameraBottom)},
        .size = size,
    };
}

[[nodiscard]] int countRect(sgl::RenderQueue& queue, const sgl::Rect<float>& expected)
{
    int matches = 0;
    for(const sgl::Entry& entry: queue.sorted())
    {
        const auto* rect = std::get_if<sgl::RectCmd>(&entry.command);
        if(rect == nullptr)
        {
            continue;
        }
        if(rect->rect.pos.x == expected.pos.x && rect->rect.pos.y == expected.pos.y
           && rect->rect.size.x == expected.size.x && rect->rect.size.y == expected.size.y)
        {
            ++matches;
        }
    }
    return matches;
}

} // namespace

TEST(PlaySceneTest, drawsPlayerAndPlatformsWithWorldYUp)
{
    Fixture f;
    f.stack.push(sgl::tower::play(f.services)());
    auto* play = dynamic_cast<sgl::tower::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);

    const float camera = play->game().cameraBottom();
    const sgl::tower::PlayerView player = play->game().player();
    const sgl::tower::PlatformView& ground = play->game().platforms().front();
    ASSERT_GT(ground.pos.y + ground.size.y, player.pos.y - 0.001f);
    ASSERT_LT(ground.pos.y + ground.size.y, player.pos.y + 0.001f);

    sgl::RenderQueue queue;
    play->render(queue);

    const sgl::Rect<float> playerRect = screenRect(player.pos, player.size, camera);
    const sgl::Rect<float> groundRect = screenRect(ground.pos, ground.size, camera);
    EXPECT_EQ(countRect(queue, playerRect), 1);
    EXPECT_EQ(countRect(queue, groundRect), 1);
    EXPECT_FLOAT_EQ(playerRect.pos.y + player.size.y, groundRect.pos.y);
    EXPECT_LT(playerRect.pos.y, groundRect.pos.y);

    bool drewHigherPlatformAbove = false;
    for(const sgl::tower::PlatformView& platform: play->game().platforms())
    {
        if(platform.pos.y + platform.size.y <= ground.pos.y + ground.size.y)
        {
            continue;
        }
        const sgl::Rect<float> higher = screenRect(platform.pos, platform.size, camera);
        EXPECT_GE(countRect(queue, higher), 1);
        EXPECT_LT(higher.pos.y, groundRect.pos.y);
        drewHigherPlatformAbove = true;
    }
    EXPECT_TRUE(drewHigherPlatformAbove);
}

TEST(PlaySceneTest, rightIsHeldAndJumpIsPressedEdge)
{
    Fixture f;
    f.services.tuning.startPlatformW = sgl::tower::kWorldW;
    f.services.tuning.iceFriction = 0.f;
    f.stack.push(sgl::tower::play(f.services)());
    auto* play = dynamic_cast<sgl::tower::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    const float originX = play->game().player().pos.x;

    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = sgl::Key::Right}, f.map);
    input.apply(sgl::KeyDown{.key = sgl::Key::Space}, f.map);
    f.stack.update(input, sgl::kTick);

    EXPECT_GT(play->game().player().pos.x, originX);
    EXPECT_FALSE(play->game().player().grounded);

    input.beginFrame();
    bool landed = false;
    for(int i = 0; i < 240 && !play->game().over(); ++i)
    {
        f.stack.update(input, sgl::kTick);
        if(play->game().player().grounded)
        {
            landed = true;
            break;
        }
    }
    ASSERT_TRUE(landed);

    f.stack.update(input, sgl::kTick);
    EXPECT_TRUE(play->game().player().grounded);

    input.apply(sgl::KeyDown{.key = sgl::Key::Space}, f.map);
    f.stack.update(input, sgl::kTick);
    EXPECT_FALSE(play->game().player().grounded);
}

TEST(PlaySceneTest, diedReachesGameOver)
{
    Fixture f;
    f.services.tuning.startPlatformW = 40.f;
    f.services.tuning.playerW = 40.f;
    f.services.tuning.moveAccel = 8000.f;
    f.services.tuning.maxRunSpeed = 600.f;
    f.services.tuning.iceFriction = 0.f;
    f.services.tuning.gravity = 4000.f;
    f.stack.push(sgl::tower::play(f.services)());

    auto* play = dynamic_cast<sgl::tower::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_FALSE(play->game().over());

    const sgl::InputState right = [&f] {
        sgl::InputState input;
        input.apply(sgl::KeyDown{.key = sgl::Key::Right}, f.map);
        return input;
    }();

    for(int i = 0; i < 600 && dynamic_cast<sgl::tower::PlayScene*>(f.stack.top()) != nullptr; ++i)
    {
        f.stack.update(right, sgl::kTick);
    }

    auto* over = dynamic_cast<sgl::tower::GameOverScene*>(f.stack.top());
    ASSERT_NE(over, nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_EQ(over->score(), 0);
    EXPECT_FALSE(over->traits().pausable);
}
