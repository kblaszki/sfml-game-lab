#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sgl/loop/App.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/resources/ResourceError.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/sfml/SfmlClock.hpp>
#include <sgl/sfml/SfmlPlatform.hpp>
#include <tower/app/Bindings.hpp>
#include <tower/app/Scenes.hpp>
#include <tower/app/Theme.hpp>

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
        {static_cast<std::uint32_t>(sgl::tower::designWidth), static_cast<std::uint32_t>(sgl::tower::designHeight)},
        "Tower"};

    const auto fontLoaded =
        platform.assets().font("ui", std::filesystem::path{ASSET_DIR} / sgl::tower::uiFontRelativePath);
    if(!fontLoaded)
    {
        return fail(fontLoaded.error());
    }

    sgl::ActionMap map;
    const sgl::tower::TowerActions actions = sgl::tower::bindTower(map);
    const sgl::tower::AppServices services{actions, *fontLoaded, kDefaultSeed};

    sgl::SceneStack stack{sgl::tower::pause(services)};
    stack.push(sgl::tower::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
