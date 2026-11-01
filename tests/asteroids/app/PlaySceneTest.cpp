#include <asteroids/app/Bindings.hpp>
#include <asteroids/app/GameOverScene.hpp>
#include <asteroids/app/PlayScene.hpp>
#include <asteroids/app/Scenes.hpp>
#include <asteroids/sim/Tuning.hpp>
#include <asteroids/sim/Types.hpp>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneStack.hpp>

namespace
{

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::asteroids::bindAsteroids(map)}
        , font{}
        , services{actions, font, 1}
        , stack{sgl::asteroids::pause(services)}
    {
    }

    sgl::ActionMap map;
    sgl::asteroids::AsteroidsActions actions;
    sgl::FontId font;
    sgl::asteroids::AppServices services;
    sgl::SceneStack stack;
};

} // namespace

TEST(PlaySceneTest, controlsReachTheSim)
{
    Fixture f;
    f.services.tuning.initialLargeRocks = 0;
    f.stack.push(sgl::asteroids::play(f.services)());

    auto* play = dynamic_cast<sgl::asteroids::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);

    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = sgl::Key::Up}, f.map);
    input.apply(sgl::KeyDown{.key = sgl::Key::Space}, f.map);
    f.stack.update(input, sgl::Seconds{0.1f});

    EXPECT_GT(play->game().ship().vel.x, 0.f);
    EXPECT_EQ(play->game().bullets().size(), 1u);
    EXPECT_FALSE(play->game().over());
}

TEST(PlaySceneTest, drawsShipTriangleAndRocks)
{
    Fixture f;
    f.stack.push(sgl::asteroids::play(f.services)());

    auto* play = dynamic_cast<sgl::asteroids::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    const sgl::asteroids::ShipView ship = play->game().ship();
    ASSERT_FALSE(play->game().rocks().empty());

    sgl::RenderQueue queue;
    play->render(queue);

    bool foundMesh = false;
    float maxMeshX = ship.pos.x;
    int circles = 0;
    for(const sgl::Entry& entry: queue.sorted())
    {
        if(const auto* mesh = std::get_if<sgl::MeshCmd>(&entry.command))
        {
            foundMesh = true;
            EXPECT_EQ(mesh->vertices.size(), 3u);
            for(const sgl::Vertex& vertex: mesh->vertices)
            {
                if(vertex.position.x > maxMeshX)
                {
                    maxMeshX = vertex.position.x;
                }
            }
        }
        if(std::holds_alternative<sgl::CircleCmd>(entry.command))
        {
            ++circles;
        }
    }

    EXPECT_TRUE(foundMesh);
    EXPECT_GT(maxMeshX, ship.pos.x);
    EXPECT_EQ(circles, static_cast<int>(play->game().rocks().size()));
}

TEST(PlaySceneTest, diedReachesGameOver)
{
    Fixture f;
    // Larger than any wrapped distance in the 1280×720 world, so the first step overlaps a rock.
    f.services.tuning.shipRadius = 10000.f;
    f.services.tuning.initialLargeRocks = 1;
    f.stack.push(sgl::asteroids::play(f.services)());

    auto* play = dynamic_cast<sgl::asteroids::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
    EXPECT_FALSE(play->game().over());

    sgl::asteroids::GameOverScene* over = nullptr;
    for(int i = 0; i < 5 && over == nullptr; ++i)
    {
        f.stack.update(sgl::InputState{}, sgl::kTick);
        over = dynamic_cast<sgl::asteroids::GameOverScene*>(f.stack.top());
    }

    ASSERT_NE(over, nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_EQ(over->score(), 0);
    EXPECT_FALSE(over->traits().pausable);
}
