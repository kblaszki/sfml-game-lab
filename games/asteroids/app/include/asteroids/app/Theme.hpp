#pragma once

#include <asteroids/sim/Types.hpp>
#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <string_view>

namespace sgl::asteroids
{

inline constexpr float designWidth{kWorldW};
inline constexpr float designHeight{kWorldH};

inline constexpr std::string_view uiFontRelativePath{"fonts/upheavtt.ttf"};

inline constexpr sgl::Color titleCyan{120, 210, 230};
inline constexpr sgl::Color hudText{220, 235, 245};
inline constexpr sgl::Color spaceBlack{8, 10, 18};
inline constexpr sgl::Color shipColor{230, 245, 255};
inline constexpr sgl::Color rockFill{148, 156, 168};
inline constexpr sgl::Color bulletFill{255, 214, 84};
inline constexpr sgl::Color menuButton{28, 48, 72};
inline constexpr sgl::Color menuButtonFocus{48, 96, 140};
inline constexpr sgl::Color menuLabel{236, 246, 255};
inline constexpr sgl::Color overlayDim{0, 0, 0, 150};
inline constexpr sgl::Color panelBox{12, 18, 32, 230};
inline constexpr sgl::Color scorePlate{0, 0, 0, 150};

inline constexpr sgl::Vec2f scorePos{24.f, 14.f};

} // namespace sgl::asteroids
