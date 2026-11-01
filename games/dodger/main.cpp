#include <cstdint>
#include <dodger/app/Bindings.hpp>
#include <dodger/app/Scenes.hpp>
#include <dodger/app/Theme.hpp>
#include <filesystem>
#include <iostream>
#include <sgl/loop/App.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/resources/ResourceError.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/sfml/SfmlClock.hpp>
#include <sgl/sfml/SfmlPlatform.hpp>

namespace
{

constexpr std::uint64_t kDefaultSeed{42};

[[nodiscard]] int fail(sgl::ResourceError err)
{
    std::cerr << sgl::describe(err) << '\n';
    return 1;
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

    sgl::ActionMap map;
    const sgl::dodger::DodgerActions actions = sgl::dodger::bindDodger(map);
    const sgl::dodger::AppServices services{actions, fontId, kDefaultSeed};

    sgl::SceneStack stack{sgl::dodger::pause(services)};
    stack.push(sgl::dodger::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
