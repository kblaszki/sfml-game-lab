#pragma once

#include <dodger/sim/Types.hpp>
#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <string_view>

namespace sgl::dodger
{

inline constexpr float designWidth{kWorldW};
inline constexpr float designHeight{kWorldH};

inline constexpr std::string_view uiFontRelativePath{"fonts/upheavtt.ttf"};

inline constexpr sgl::Color titleAmber{242, 176, 68};
inline constexpr sgl::Color hudText{236, 228, 210};
inline constexpr sgl::Color fieldFill{18, 22, 32};
inline constexpr sgl::Color paddleFill{242, 214, 96};
inline constexpr sgl::Color hazardFill{214, 78, 52};
inline constexpr sgl::Color menuButton{48, 42, 36};
inline constexpr sgl::Color menuButtonFocus{120, 78, 36};
inline constexpr sgl::Color menuLabel{255, 244, 220};
inline constexpr sgl::Color overlayDim{0, 0, 0, 150};
inline constexpr sgl::Color panelBox{22, 18, 16, 230};
inline constexpr sgl::Color scorePlate{0, 0, 0, 150};

inline constexpr sgl::Vec2f scorePos{24.f, 14.f};

} // namespace sgl::dodger
