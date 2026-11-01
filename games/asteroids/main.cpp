#include <asteroids/app/Bindings.hpp>
#include <asteroids/app/Scenes.hpp>
#include <asteroids/app/Theme.hpp>
#include <cstdint>
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

    sgl::ActionMap map;
    const sgl::asteroids::AsteroidsActions actions = sgl::asteroids::bindAsteroids(map);
    const sgl::asteroids::AppServices services{actions, fontId, kDefaultSeed};

    sgl::SceneStack stack{sgl::asteroids::pause(services)};
    stack.push(sgl::asteroids::mainMenu(services)());

    sgl::sfml::SfmlClock clock;
    sgl::RenderQueue queue;
    sgl::App{platform, clock, std::move(map), stack, queue}.run();
    return 0;
}
