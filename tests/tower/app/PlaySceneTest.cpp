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
#include <sgl/resources/Atlas.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <system_error>
#include <tower/app/Bindings.hpp>
#include <tower/app/GameOverScene.hpp>
#include <tower/app/PlayScene.hpp>
#include <tower/app/Scenes.hpp>
#include <tower/app/Sounds.hpp>
#include <tower/sim/Tuning.hpp>
#include <tower/sim/Types.hpp>
#include <variant>
#include <vector>

namespace
{

sgl::tower::SoundIds testSoundIds()
{
    return sgl::tower::SoundIds{
        .jump = sgl::SoundId{.id = 1},
        .land = sgl::SoundId{.id = 2},
        .die = sgl::SoundId{.id = 3},
        .wall = sgl::SoundId{.id = 4},
        .bonus = sgl::SoundId{.id = 5},
    };
}

[[nodiscard]] std::filesystem::path uniqueScoresPath()
{
    static int seq = 0;
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
           / ("tower_play_" + std::to_string(stamp) + "_" + std::to_string(++seq) + ".scores");
}

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::tower::bindTower(map)}
        , font{}
        , audio{}
        , sounds{testSoundIds()}
        , scoresPath{uniqueScoresPath()}
        , highScores{sgl::tower::kHighScoreCapacity}
        , services{actions, font, 1, audio, sounds, scoresPath, highScores}
        , stack{sgl::tower::pause(services)}
    {
    }

    ~Fixture()
    {
        std::error_code ec;
        std::filesystem::remove(scoresPath, ec);
    }

    sgl::ActionMap map;
    sgl::tower::TowerActions actions;
    sgl::FontId font;
    sgl::AudioSpy audio;
    sgl::tower::SoundIds sounds;
    std::filesystem::path scoresPath;
    sgl::HighScoreTable highScores;
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

TEST(PlaySceneTest, rightAndJumpAreHeld)
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
    ASSERT_FALSE(f.audio.plays.empty());
    EXPECT_EQ(f.audio.plays.front().id, f.sounds.jump);

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
    bool heardLand = false;
    for(const sgl::AudioSpy::Play& playSound: f.audio.plays)
    {
        if(playSound.id == f.sounds.land)
        {
            heardLand = true;
        }
    }
    EXPECT_TRUE(heardLand);

    // Space still held → autojump on the landing frame's next step.
    const std::size_t playsBeforeAutojump = f.audio.plays.size();
    f.stack.update(input, sgl::kTick);
    EXPECT_FALSE(play->game().player().grounded);
    ASSERT_GT(f.audio.plays.size(), playsBeforeAutojump);
    EXPECT_EQ(f.audio.plays.back().id, f.sounds.jump);

    // Release jump; after the next landing, stay grounded.
    input.apply(sgl::KeyUp{.key = sgl::Key::Space}, f.map);
    input.beginFrame();
    landed = false;
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
    bool heardDie = false;
    for(const sgl::AudioSpy::Play& playSound: f.audio.plays)
    {
        if(playSound.id == f.sounds.die)
        {
            heardDie = true;
        }
    }
    EXPECT_TRUE(heardDie);
}

sgl::tower::SpriteBank testSprites()
{
    sgl::AtlasManifest manifest{std::vector<sgl::AtlasEntry>{
        sgl::AtlasEntry{.name = "bg/sky", .rect = sgl::AtlasRect{.x = 0, .y = 0, .w = 32, .h = 32}},
        sgl::AtlasEntry{.name = "player/idle", .rect = sgl::AtlasRect{.x = 0, .y = 32, .w = 10, .h = 14}},
        sgl::AtlasEntry{.name = "player/jump", .rect = sgl::AtlasRect{.x = 10, .y = 32, .w = 10, .h = 14}},
        sgl::AtlasEntry{.name = "plat/ice", .rect = sgl::AtlasRect{.x = 20, .y = 32, .w = 8, .h = 8}},
    }};
    return sgl::tower::SpriteBank{sgl::TextureId{7}, std::move(manifest)};
}

TEST(PlaySceneTest, spritesReplaceRectsWhenRegionsExist)
{
    Fixture f;
    f.services.sprites = testSprites();
    f.stack.push(sgl::tower::play(f.services)());
    auto* play = dynamic_cast<sgl::tower::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);

    const sgl::tower::PlayerView player = play->game().player();
    const sgl::Rect<float> playerRect = screenRect(player.pos, player.size, play->game().cameraBottom());

    sgl::RenderQueue queue;
    play->render(queue);

    int sprites = 0;
    EXPECT_EQ(countRect(queue, playerRect), 0);
    for(const sgl::Entry& entry: queue.sorted())
    {
        if(std::holds_alternative<sgl::SpriteCmd>(entry.command))
        {
            ++sprites;
        }
    }
    EXPECT_GE(sprites, 3);
}

TEST(PlaySceneTest, leftFacingFlipsPlayerSprite)
{
    Fixture f;
    f.services.tuning.startPlatformW = sgl::tower::kWorldW;
    f.services.tuning.iceFriction = 0.f;
    f.services.sprites = testSprites();
    f.stack.push(sgl::tower::play(f.services)());
    auto* play = dynamic_cast<sgl::tower::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);

    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = sgl::Key::Left}, f.map);
    for(int i = 0; i < 5; ++i)
    {
        f.stack.update(input, sgl::kTick);
        input.beginFrame();
    }
    ASSERT_LT(play->game().player().vel.x, 0.f);
    ASSERT_TRUE(play->game().player().grounded);

    const sgl::tower::PlayerView player = play->game().player();
    const sgl::Rect<float> playerRect = screenRect(player.pos, player.size, play->game().cameraBottom());
    const auto idle =
        f.services.sprites.sprite("player/idle", playerRect.pos, playerRect.size, sgl::Color{255, 255, 255, 255});
    ASSERT_TRUE(idle.has_value());
    const float destW = idle->source.size.x * idle->scale.x;

    sgl::RenderQueue queue;
    play->render(queue);
    const sgl::SpriteCmd* drawn = nullptr;
    for(const sgl::Entry& entry: queue.sorted())
    {
        if(const auto* sprite = std::get_if<sgl::SpriteCmd>(&entry.command))
        {
            if(sprite->source.pos.x == idle->source.pos.x && sprite->source.size.x == idle->source.size.x)
            {
                drawn = sprite;
            }
        }
    }
    ASSERT_NE(drawn, nullptr);
    EXPECT_FLOAT_EQ(drawn->scale.x, -idle->scale.x);
    EXPECT_FLOAT_EQ(drawn->position.x, playerRect.pos.x + destW);
    EXPECT_FLOAT_EQ(drawn->position.y, playerRect.pos.y);
}
