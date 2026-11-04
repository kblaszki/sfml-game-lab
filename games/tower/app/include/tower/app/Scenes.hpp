#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <sgl/audio/AudioI.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneRequest.hpp>
#include <tower/app/Bindings.hpp>
#include <tower/app/Sounds.hpp>
#include <tower/app/SpriteBank.hpp>
#include <tower/sim/Tuning.hpp>

namespace sgl::tower
{

inline constexpr std::size_t kHighScoreCapacity{5};

// References are owned by main() for the whole App::run(); scenes must not outlive that.
// SpriteBank is a value (empty when atlas files are missing).
struct AppServices
{
    const TowerActions& actions;
    sgl::FontId font;
    std::uint64_t seed{};
    sgl::AudioI& audio;
    SoundIds sounds{};
    std::filesystem::path scoresPath{};
    sgl::HighScoreTable& highScores;
    Tuning tuning{};
    SpriteBank sprites{};
};

[[nodiscard]] sgl::SceneFactory mainMenu(const AppServices& services);
[[nodiscard]] sgl::SceneFactory play(const AppServices& services);
[[nodiscard]] sgl::SceneFactory pause(const AppServices& services);
[[nodiscard]] sgl::SceneFactory gameOver(const AppServices& services, int score);

} // namespace sgl::tower
