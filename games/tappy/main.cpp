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
#include <tappy/app/Bindings.hpp>
#include <tappy/app/Scenes.hpp>
#include <tappy/app/Sounds.hpp>
#include <tappy/app/SpriteBank.hpp>
#include <tappy/app/Theme.hpp>

namespace
{

constexpr std::uint64_t kDefaultSeed{42};

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

void assignSound(sgl::tappy::SoundIds& ids, std::string_view key, sgl::SoundId id)
{
    if(key == "flap")
    {
        ids.flap = id;
    }
    else if(key == "clear")
    {
        ids.clear = id;
    }
    else if(key == "die")
    {
        ids.die = id;
    }
}

} // namespace

int main()
{
    sgl::sfml::SfmlPlatform platform{
        {static_cast<std::uint32_t>(sgl::tappy::designWidth), static_cast<std::uint32_t>(sgl::tappy::designHeight)},
        "Tappy"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::tappy::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    auto& sfmlAudio = platform.audio();
    sgl::tappy::SoundIds sounds{};
    for(const sgl::tappy::SoundSpec& spec: sgl::tappy::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        assignSound(sounds, spec.key, *uploaded);
    }

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("tappy"))
    {
        scoresPath = *dir / "tappy.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "tappy.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::tappy::kHighScoreCapacity);

    const auto atlasPng = std::filesystem::path{ASSET_DIR} / "tappy" / "atlas.png";
    const auto atlasTxt = std::filesystem::path{ASSET_DIR} / "tappy" / "atlas.txt";
    sgl::tappy::SpriteBank sprites{};
    if(std::filesystem::exists(atlasPng) && std::filesystem::exists(atlasTxt))
    {
        if(const auto text = sgl::readTextFile(atlasTxt))
        {
            if(const auto manifest = sgl::parseAtlas(*text))
            {
                if(const auto id = platform.assets().textureFile("tappy/atlas", atlasPng, true))
                {
                    sprites = sgl::tappy::SpriteBank{*id, *manifest};
                }
            }
        }
    }

    sgl::ActionMap map;
    const sgl::tappy::TappyActions actions = sgl::tappy::bindTappy(map);
    const sgl::tappy::AppServices services{
        actions, fontId, kDefaultSeed, platform.audio(), sounds, scoresPath, highScores, {}, std::move(sprites)};

    sgl::SceneStack stack{sgl::tappy::pause(services)};
    stack.push(sgl::tappy::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
