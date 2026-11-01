#pragma once

#include <cstdint>
#include <dodger/app/Bindings.hpp>
#include <dodger/sim/Tuning.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/scene/SceneRequest.hpp>

namespace sgl::dodger
{

// References are owned by main() for the whole App::run(); scenes must not outlive that.
struct AppServices
{
    const DodgerActions& actions;
    sgl::FontId font;
    std::uint64_t seed{};
    Tuning tuning{};
};

[[nodiscard]] sgl::SceneFactory mainMenu(const AppServices& services);
[[nodiscard]] sgl::SceneFactory play(const AppServices& services);
[[nodiscard]] sgl::SceneFactory pause(const AppServices& services);
[[nodiscard]] sgl::SceneFactory gameOver(const AppServices& services, int score);

} // namespace sgl::dodger
