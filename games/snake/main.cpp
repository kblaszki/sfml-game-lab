#include <cstdint>
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
#include <snake/app/Bindings.hpp>
#include <snake/app/Scenes.hpp>
#include <snake/app/Sounds.hpp>
#include <snake/app/Theme.hpp>
#include <string_view>

namespace
{

constexpr std::uint64_t kDefaultSeed{42};

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

void assignSound(sgl::snake::SoundIds& ids, std::string_view key, sgl::SoundId id)
{
    if(key == "eat")
    {
        ids.eat = id;
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
        {static_cast<std::uint32_t>(sgl::snake::designWidth), static_cast<std::uint32_t>(sgl::snake::designHeight)},
        "Snake"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::snake::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    auto& sfmlAudio = platform.audio();
    sgl::snake::SoundIds sounds{};
    for(const sgl::snake::SoundSpec& spec: sgl::snake::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        assignSound(sounds, spec.key, *uploaded);
    }

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("snake"))
    {
        scoresPath = *dir / "snake.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "snake.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::snake::kHighScoreCapacity);

    sgl::ActionMap map;
    const sgl::snake::SnakeActions actions = sgl::snake::bindSnake(map);
    const sgl::snake::AppServices services{
        actions, fontId, kDefaultSeed, platform.audio(), sounds, scoresPath, highScores};

    sgl::SceneStack stack{sgl::snake::pause(services)};
    stack.push(sgl::snake::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
