#include <cstdint>
#include <dodger/app/Bindings.hpp>
#include <dodger/app/Scenes.hpp>
#include <dodger/app/Sounds.hpp>
#include <dodger/app/Theme.hpp>
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

namespace
{

constexpr std::uint64_t kDefaultSeed{42};

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

void assignSound(sgl::dodger::SoundIds& ids, std::string_view key, sgl::SoundId id)
{
    if(key == "die")
    {
        ids.die = id;
    }
}

} // namespace

int main()
{
    sgl::sfml::SfmlPlatform platform{
        {static_cast<std::uint32_t>(sgl::dodger::designWidth), static_cast<std::uint32_t>(sgl::dodger::designHeight)},
        "Dodger"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::dodger::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    auto& sfmlAudio = platform.audio();
    sgl::dodger::SoundIds sounds{};
    for(const sgl::dodger::SoundSpec& spec: sgl::dodger::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        assignSound(sounds, spec.key, *uploaded);
    }

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("dodger"))
    {
        scoresPath = *dir / "dodger.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "dodger.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::dodger::kHighScoreCapacity);

    sgl::ActionMap map;
    const sgl::dodger::DodgerActions actions = sgl::dodger::bindDodger(map);
    const sgl::dodger::AppServices services{
        actions, fontId, kDefaultSeed, platform.audio(), sounds, scoresPath, highScores};

    sgl::SceneStack stack{sgl::dodger::pause(services)};
    stack.push(sgl::dodger::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
