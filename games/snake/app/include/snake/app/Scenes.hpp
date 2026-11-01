#pragma once

#include <cstdint>
#include <sgl/render/AssetIds.hpp>
#include <sgl/scene/SceneRequest.hpp>
#include <snake/app/Bindings.hpp>

namespace sgl::snake
{

// References are owned by main() for the whole App::run(); scenes must not outlive that.
struct AppServices
{
    const SnakeActions& actions;
    sgl::FontId font;
    std::uint64_t seed{};
};

[[nodiscard]] sgl::SceneFactory mainMenu(const AppServices& services);
[[nodiscard]] sgl::SceneFactory play(const AppServices& services);
[[nodiscard]] sgl::SceneFactory pause(const AppServices& services);
[[nodiscard]] sgl::SceneFactory gameOver(const AppServices& services, int score);

} // namespace sgl::snake
