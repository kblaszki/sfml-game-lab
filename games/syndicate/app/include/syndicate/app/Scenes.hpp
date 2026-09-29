#pragma once

#include <cstddef>
#include <sgl/scene/SceneRequest.hpp>
#include <string_view>
#include <syndicate/app/PlayScene.hpp>
#include <syndicate/sim/GameState.hpp>
#include <vector>

namespace sgl::syndicate
{

inline constexpr std::size_t kHighScoreCapacity{5};
inline constexpr std::string_view uiFontRelativePath{"fonts/upheavtt.ttf"};
inline constexpr std::string_view bodyFontRelativePath{"fonts/VCR_OSD_MONO_1.001.ttf"};

[[nodiscard]] sgl::SceneFactory mainMenu(const SyndicateServices& services);
[[nodiscard]] sgl::SceneFactory setup(const SyndicateServices& services);
[[nodiscard]] sgl::SceneFactory play(const SyndicateServices& services,
                                     MatchConfig config,
                                     std::vector<Seat> seats);
[[nodiscard]] sgl::SceneFactory pause(const SyndicateServices& services);
[[nodiscard]] sgl::SceneFactory makeGameOver(const SyndicateServices& services, GameOver over);

} // namespace sgl::syndicate
