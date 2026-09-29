#pragma once

#include <arkanoid/app/Bindings.hpp>
#include <arkanoid/app/SceneRender.hpp>
#include <arkanoid/app/Sounds.hpp>
#include <arkanoid/sim/State.hpp>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <sgl/audio/AudioI.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/scene/SceneRequest.hpp>

namespace sgl::arkanoid
{

inline constexpr std::size_t kHighScoreCapacity{5};

enum class Outcome
{
    Won,
    Lost
};

// AppServices holds references owned by main() for the whole App::run(); scenes must not outlive that.
struct AppServices
{
    const Actions& actions;
    const TextureIds& textures;
    sgl::FontId font;
    sgl::AudioI& audio;
    SoundIds sounds{};
    sgl::HighScoreTable& highScores;
    std::filesystem::path scoresPath;
};

[[nodiscard]] sgl::SceneFactory mainMenu(const AppServices& services);
[[nodiscard]] sgl::SceneFactory gameplay(const AppServices& services, StageId stage);
[[nodiscard]] sgl::SceneFactory pause(const AppServices& services);
[[nodiscard]] sgl::SceneFactory result(const AppServices& services,
                                       Outcome outcome,
                                       std::uint32_t score,
                                       std::optional<std::size_t> rank,
                                       bool saveFailed);

} // namespace sgl::arkanoid
