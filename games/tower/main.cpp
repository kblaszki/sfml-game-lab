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
#include <tower/app/Bindings.hpp>
#include <tower/app/Scenes.hpp>
#include <tower/app/Sounds.hpp>
#include <tower/app/SpriteBank.hpp>
#include <tower/app/Theme.hpp>

namespace
{

constexpr std::uint64_t kDefaultSeed{42};

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

void assignSound(sgl::tower::SoundIds& ids, std::string_view key, sgl::SoundId id)
{
    if(key == "jump")
    {
        ids.jump = id;
    }
    else if(key == "land")
    {
        ids.land = id;
    }
    else if(key == "die")
    {
        ids.die = id;
    }
    else if(key == "wall")
    {
        ids.wall = id;
    }
    else if(key == "bonus")
    {
        ids.bonus = id;
    }
}

} // namespace

int main()
{
    sgl::sfml::SfmlPlatform platform{
        {static_cast<std::uint32_t>(sgl::tower::designWidth), static_cast<std::uint32_t>(sgl::tower::designHeight)},
        "Tower"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::tower::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    auto& sfmlAudio = platform.audio();
    sgl::tower::SoundIds sounds{};
    for(const sgl::tower::SoundSpec& spec: sgl::tower::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        assignSound(sounds, spec.key, *uploaded);
    }

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("tower"))
    {
        scoresPath = *dir / "tower.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "tower.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::tower::kHighScoreCapacity);

    const auto atlasPng = std::filesystem::path{ASSET_DIR} / "tower" / "atlas.png";
    const auto atlasTxt = std::filesystem::path{ASSET_DIR} / "tower" / "atlas.txt";
    sgl::tower::SpriteBank sprites{};
    if(std::filesystem::exists(atlasPng) && std::filesystem::exists(atlasTxt))
    {
        if(const auto text = sgl::readTextFile(atlasTxt))
        {
            if(const auto manifest = sgl::parseAtlas(*text))
            {
                if(const auto id = platform.assets().textureFile("tower/atlas", atlasPng, true))
                {
                    sprites = sgl::tower::SpriteBank{*id, *manifest};
                }
            }
        }
    }

    sgl::ActionMap map;
    const sgl::tower::TowerActions actions = sgl::tower::bindTower(map);
    const sgl::tower::AppServices services{
        actions, fontId, kDefaultSeed, platform.audio(), sounds, scoresPath, highScores, {}, std::move(sprites)};

    sgl::SceneStack stack{sgl::tower::pause(services)};
    stack.push(sgl::tower::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
