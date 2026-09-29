#include <chrono>
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
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/Scenes.hpp>
#include <syndicate/app/Sounds.hpp>
#include <syndicate/app/SpriteBank.hpp>

namespace
{

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

void assignSound(sgl::syndicate::SoundIds& ids, std::string_view key, sgl::SoundId id)
{
    if(key == "roll")
    {
        ids.roll = id;
    }
    else if(key == "fee")
    {
        ids.fee = id;
    }
    else if(key == "auction")
    {
        ids.auction = id;
    }
    else if(key == "card")
    {
        ids.card = id;
    }
    else if(key == "market")
    {
        ids.market = id;
    }
}

} // namespace

int main()
{
    sgl::sfml::SfmlPlatform platform{{1280u, 720u}, "Syndicate"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::syndicate::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    const auto bodyLoaded =
        platform.assets().font("body", std::filesystem::path{ASSET_DIR} / sgl::syndicate::bodyFontRelativePath);
    if(!bodyLoaded)
    {
        return fail(bodyLoaded.error());
    }
    const sgl::FontId bodyFontId = *bodyLoaded;

    const auto atlasPng = std::filesystem::path{ASSET_DIR} / "syndicate" / "atlas.png";
    const auto atlasTxt = std::filesystem::path{ASSET_DIR} / "syndicate" / "atlas.txt";
    sgl::syndicate::SpriteBank sprites{};
    if(std::filesystem::exists(atlasPng) && std::filesystem::exists(atlasTxt))
    {
        if(const auto text = sgl::readTextFile(atlasTxt))
        {
            if(const auto manifest = sgl::parseAtlas(*text))
            {
                if(const auto id = platform.assets().textureFile("syndicate/atlas", atlasPng, true))
                {
                    sprites = sgl::syndicate::SpriteBank{*id, *manifest};
                }
            }
        }
    }

    auto& sfmlAudio = platform.audio();
    sgl::syndicate::SoundIds sounds{};
    for(const sgl::syndicate::SoundSpec& spec: sgl::syndicate::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        assignSound(sounds, spec.key, *uploaded);
    }

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("syndicate"))
    {
        scoresPath = *dir / "syndicate.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "syndicate.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::syndicate::kHighScoreCapacity);

    sgl::ActionMap map;
    const sgl::syndicate::SyndicateActions actions = sgl::syndicate::bindSyndicate(map);
    const auto seed = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const sgl::syndicate::SyndicateServices services{
        actions, fontId, seed, platform.audio(), sounds, scoresPath, highScores, sprites, bodyFontId};

    sgl::SceneStack stack{sgl::syndicate::pause(services)};
    stack.push(sgl::syndicate::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
