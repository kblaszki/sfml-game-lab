#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <sgl/audio/AudioI.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneRequest.hpp>
#include <tetris/app/Bindings.hpp>
#include <tetris/app/Sounds.hpp>

namespace sgl::tetris
{

inline constexpr std::size_t kHighScoreCapacity{5};

// References are owned by main() for the whole App::run(); scenes must not outlive that.
struct AppServices
{
    const Actions& actions;
    sgl::FontId font;
    std::uint64_t seed{};
    sgl::AudioI& audio;
    SoundIds sounds{};
    std::filesystem::path scoresPath{};
    sgl::HighScoreTable& highScores;
};

[[nodiscard]] sgl::SceneFactory mainMenu(const AppServices& services);
[[nodiscard]] sgl::SceneFactory play(const AppServices& services);
[[nodiscard]] sgl::SceneFactory pause(const AppServices& services);
[[nodiscard]] sgl::SceneFactory gameOver(const AppServices& services, std::uint32_t score);

} // namespace sgl::tetris
