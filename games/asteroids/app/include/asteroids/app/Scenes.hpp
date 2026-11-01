#pragma once

#include <asteroids/app/Bindings.hpp>
#include <asteroids/app/Sounds.hpp>
#include <asteroids/sim/Tuning.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <sgl/audio/AudioI.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneRequest.hpp>

namespace sgl::asteroids
{

inline constexpr std::size_t kHighScoreCapacity{5};

// References are owned by main() for the whole App::run(); scenes must not outlive that.
struct AppServices
{
    const AsteroidsActions& actions;
    sgl::FontId font;
    std::uint64_t seed{};
    sgl::AudioI& audio;
    SoundIds sounds{};
    std::filesystem::path scoresPath{};
    sgl::HighScoreTable& highScores;
    Tuning tuning{};
};

[[nodiscard]] sgl::SceneFactory mainMenu(const AppServices& services);
[[nodiscard]] sgl::SceneFactory play(const AppServices& services);
[[nodiscard]] sgl::SceneFactory pause(const AppServices& services);
[[nodiscard]] sgl::SceneFactory gameOver(const AppServices& services, int score);

} // namespace sgl::asteroids
