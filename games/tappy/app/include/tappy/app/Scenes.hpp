#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <sgl/audio/AudioI.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneRequest.hpp>
#include <tappy/app/Bindings.hpp>
#include <tappy/app/Sounds.hpp>
#include <tappy/app/SpriteBank.hpp>
#include <tappy/sim/Tuning.hpp>

namespace sgl::tappy
{

inline constexpr std::size_t kHighScoreCapacity{5};

struct AppServices
{
    const TappyActions& actions;
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

} // namespace sgl::tappy
