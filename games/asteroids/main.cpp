#include <asteroids/app/Bindings.hpp>
#include <asteroids/app/Scenes.hpp>
#include <asteroids/app/Sounds.hpp>
#include <asteroids/app/SpriteBank.hpp>
#include <asteroids/app/Theme.hpp>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sgl/loop/App.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/resources/Atlas.hpp>
#include <sgl/resources/ResourceError.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/save/SaveFile.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/sfml/SfmlAudio.hpp>
#include <sgl/sfml/SfmlClock.hpp>
#include <sgl/sfml/SfmlPlatform.hpp>
#include <string_view>

namespace
{

constexpr std::uint64_t kDefaultSeed{42};

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

void assignSound(sgl::asteroids::SoundIds& ids, std::string_view key, sgl::SoundId id)
{
    if(key == "fire")
    {
        ids.fire = id;
    }
    else if(key == "hit")
    {
        ids.hit = id;
    }
    else if(key == "die")
    {
        ids.die = id;
    }
}

} // namespace

int main()
{
    sgl::sfml::SfmlPlatform platform{{static_cast<std::uint32_t>(sgl::asteroids::designWidth),
                                      static_cast<std::uint32_t>(sgl::asteroids::designHeight)},
                                     "Asteroids"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::asteroids::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    auto& sfmlAudio = platform.audio();
    sgl::asteroids::SoundIds sounds{};
    for(const sgl::asteroids::SoundSpec& spec: sgl::asteroids::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        assignSound(sounds, spec.key, *uploaded);
    }

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("asteroids"))
    {
        scoresPath = *dir / "asteroids.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "asteroids.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::asteroids::kHighScoreCapacity);

    const auto atlasPng = std::filesystem::path{ASSET_DIR} / "asteroids" / "atlas.png";
    const auto atlasTxt = std::filesystem::path{ASSET_DIR} / "asteroids" / "atlas.txt";
    sgl::asteroids::SpriteBank sprites{};
    if(std::filesystem::exists(atlasPng) && std::filesystem::exists(atlasTxt))
    {
        if(const auto text = sgl::readTextFile(atlasTxt))
        {
            if(const auto manifest = sgl::parseAtlas(*text))
            {
                if(const auto id = platform.assets().textureFile("asteroids/atlas", atlasPng, true))
                {
                    sprites = sgl::asteroids::SpriteBank{*id, *manifest};
                }
            }
        }
    }

    sgl::ActionMap map;
    const sgl::asteroids::AsteroidsActions actions = sgl::asteroids::bindAsteroids(map);
    const sgl::asteroids::AppServices services{
        actions, fontId, kDefaultSeed, platform.audio(), sounds, scoresPath, highScores, {}, std::move(sprites)};

    sgl::SceneStack stack{sgl::asteroids::pause(services)};
    stack.push(sgl::asteroids::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
