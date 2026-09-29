#include <arkanoid/app/Assets.hpp>
#include <arkanoid/app/Bindings.hpp>
#include <arkanoid/app/SceneRender.hpp>
#include <arkanoid/app/Scenes.hpp>
#include <arkanoid/app/Sounds.hpp>
#include <array>
#include <filesystem>
#include <iostream>
#include <sgl/loop/App.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/resources/ResourceError.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/save/SaveFile.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/sfml/SfmlAudio.hpp>
#include <sgl/sfml/SfmlClock.hpp>
#include <sgl/sfml/SfmlPlatform.hpp>
#include <string_view>
#include <utility>

namespace
{

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

} // namespace

int main()
{
    sgl::sfml::SfmlPlatform platform{{1280u, 720u}, "Arkanoid"};

    sgl::arkanoid::TextureIds textures{};
    const std::array<std::pair<std::string_view, sgl::TextureId*>, 8> textureSlots{{
        {sgl::arkanoid::AssetKeys::background, &textures.background},
        {sgl::arkanoid::AssetKeys::brick, &textures.brick},
        {sgl::arkanoid::AssetKeys::paddle, &textures.paddle},
        {sgl::arkanoid::AssetKeys::ball, &textures.ball},
        {sgl::arkanoid::AssetKeys::capsuleWide, textures.capsule.data()},
        {sgl::arkanoid::AssetKeys::capsuleMulti, &textures.capsule[1]},
        {sgl::arkanoid::AssetKeys::capsuleSlow, &textures.capsule[2]},
        {sgl::arkanoid::AssetKeys::capsuleExtra, &textures.capsule[3]},
    }};

    for(const sgl::arkanoid::TextureSpec& spec: sgl::arkanoid::textureSpecs())
    {
        const auto loaded = platform.assets().texture(spec.key, spec.build());
        if(!loaded)
        {
            return fail(loaded.error());
        }

        for(const auto& [key, slot]: textureSlots)
        {
            if(key == spec.key)
            {
                *slot = *loaded;
                break;
            }
        }
    }

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::arkanoid::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    auto& sfmlAudio = platform.audio();
    sgl::arkanoid::SoundIds sounds{};
    const std::array<std::pair<std::string_view, sgl::SoundId*>, 9> soundSlots{{
        {"paddle", &sounds.paddle},
        {"brick", &sounds.brick},
        {"wall", &sounds.wall},
        {"launch", &sounds.launch},
        {"powerUp", &sounds.powerUp},
        {"ballLost", &sounds.ballLost},
        {"lifeLost", &sounds.lifeLost},
        {"stageClear", &sounds.stageClear},
        {"gameOver", &sounds.gameOver},
    }};
    for(const sgl::arkanoid::SoundSpec& spec: sgl::arkanoid::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        for(const auto& [key, slot]: soundSlots)
        {
            if(key == spec.key)
            {
                *slot = *uploaded;
                break;
            }
        }
    }

    const sgl::arkanoid::Actions actions = sgl::arkanoid::makeActions();
    sgl::ActionMap map = sgl::arkanoid::defaultBindings(actions);

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("sfml-game-lab"))
    {
        scoresPath = *dir / "arkanoid.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "arkanoid.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::arkanoid::kHighScoreCapacity);

    const sgl::arkanoid::AppServices services{
        actions, textures, fontId, platform.audio(), sounds, highScores, scoresPath};

    sgl::SceneStack stack{sgl::arkanoid::pause(services)};
    stack.push(sgl::arkanoid::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
