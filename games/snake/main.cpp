#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sgl/loop/App.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/resources/ResourceError.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/sfml/SfmlClock.hpp>
#include <sgl/sfml/SfmlPlatform.hpp>
#include <snake/app/Bindings.hpp>
#include <snake/app/Scenes.hpp>
#include <snake/app/Theme.hpp>

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
        {static_cast<std::uint32_t>(sgl::snake::designWidth), static_cast<std::uint32_t>(sgl::snake::designHeight)},
        "Snake"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::snake::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }
    const sgl::FontId fontId = *fontLoaded;

    sgl::ActionMap map;
    const sgl::snake::SnakeActions actions = sgl::snake::bindSnake(map);
    const sgl::snake::AppServices services{actions, fontId, kDefaultSeed};

    sgl::SceneStack stack{sgl::snake::pause(services)};
    stack.push(sgl::snake::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
