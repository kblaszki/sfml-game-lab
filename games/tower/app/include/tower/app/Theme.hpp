#pragma once

#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <string_view>
#include <tower/sim/Types.hpp>

namespace sgl::tower
{

inline constexpr float designWidth{kWorldW};
inline constexpr float designHeight{kViewH};

inline constexpr std::string_view uiFontRelativePath{"fonts/upheavtt.ttf"};

inline constexpr sgl::Color titleIce{186, 220, 238};
inline constexpr sgl::Color hudText{226, 232, 236};
inline constexpr sgl::Color fieldFill{16, 22, 34};
inline constexpr sgl::Color platformFill{132, 124, 112};
inline constexpr sgl::Color playerFill{242, 196, 92};
inline constexpr sgl::Color bonusSpeedFill{80, 200, 220};
inline constexpr sgl::Color bonusJumpFill{240, 200, 70};
inline constexpr sgl::Color bonusPointsFill{220, 90, 180};
inline constexpr sgl::Color menuButton{36, 44, 58};
inline constexpr sgl::Color menuButtonFocus{58, 96, 128};
inline constexpr sgl::Color menuLabel{236, 242, 246};
inline constexpr sgl::Color overlayDim{0, 0, 0, 150};
inline constexpr sgl::Color panelBox{18, 24, 34, 230};
inline constexpr sgl::Color scorePlate{0, 0, 0, 150};

inline constexpr sgl::Vec2f scorePos{24.f, 14.f};

} // namespace sgl::tower
