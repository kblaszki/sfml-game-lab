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
#include <tetris/app/Bindings.hpp>
#include <tetris/app/Scenes.hpp>
#include <tetris/app/Sounds.hpp>
#include <tetris/app/Theme.hpp>

namespace
{

constexpr std::uint64_t kDefaultSeed{42};

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
}

void assignSound(sgl::tetris::SoundIds& ids, std::string_view key, sgl::SoundId id)
{
    if(key == "move")
    {
        ids.move = id;
    }
    else if(key == "rotate")
    {
        ids.rotate = id;
    }
    else if(key == "lock")
    {
        ids.lock = id;
    }
    else if(key == "line1")
    {
        ids.line1 = id;
    }
    else if(key == "line2")
    {
        ids.line2 = id;
    }
    else if(key == "line3")
    {
        ids.line3 = id;
    }
    else if(key == "tetris")
    {
        ids.tetris = id;
    }
    else if(key == "levelUp")
    {
        ids.levelUp = id;
    }
    else if(key == "gameOver")
    {
        ids.gameOver = id;
    }
}

} // namespace

int main()
{
    sgl::sfml::SfmlPlatform platform{{1280u, 720u}, "Tetris"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::tetris::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    auto& sfmlAudio = platform.audio();
    sgl::tetris::SoundIds sounds{};
    for(const sgl::tetris::SoundSpec& spec: sgl::tetris::soundSpecs())
    {
        const auto uploaded = sfmlAudio.upload(spec.key, spec.pcm);
        if(!uploaded)
        {
            return fail(uploaded.error());
        }
        assignSound(sounds, spec.key, *uploaded);
    }

    std::filesystem::path scoresPath;
    if(const auto dir = sgl::userDataDir("sfml-game-lab"))
    {
        scoresPath = *dir / "tetris.scores";
    }
    else
    {
        scoresPath = std::filesystem::temp_directory_path() / "tetris.scores";
    }
    sgl::HighScoreTable highScores = sgl::ScoreStore::load(scoresPath, sgl::tetris::kHighScoreCapacity);

    const sgl::tetris::Actions actions = sgl::tetris::makeActions();
    sgl::ActionMap map = sgl::tetris::defaultBindings(actions);
    const sgl::tetris::AppServices services{
        actions, fontId, kDefaultSeed, platform.audio(), sounds, scoresPath, highScores};

    sgl::SceneStack stack{sgl::tetris::pause(services)};
    stack.push(sgl::tetris::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
