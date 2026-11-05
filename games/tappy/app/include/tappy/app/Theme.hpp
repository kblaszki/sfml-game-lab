#pragma once

#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <string_view>
#include <tappy/sim/Types.hpp>

namespace sgl::tappy
{

inline constexpr float designWidth{kWorldW};
inline constexpr float designHeight{kWorldH};

inline constexpr std::string_view uiFontRelativePath{"fonts/upheavtt.ttf"};

inline constexpr sgl::Color titleSky{120, 190, 230};
inline constexpr sgl::Color hudText{236, 244, 250};
inline constexpr sgl::Color fieldFill{90, 180, 230};
inline constexpr sgl::Color hillsFill{70, 150, 100};
inline constexpr sgl::Color playerFill{70, 140, 220};
inline constexpr sgl::Color gateFill{110, 90, 70};
inline constexpr sgl::Color menuButton{36, 56, 78};
inline constexpr sgl::Color menuButtonFocus{58, 110, 160};
inline constexpr sgl::Color menuLabel{236, 246, 255};
inline constexpr sgl::Color overlayDim{0, 0, 0, 150};
inline constexpr sgl::Color panelBox{18, 28, 40, 230};
inline constexpr sgl::Color scorePlate{0, 0, 0, 150};

inline constexpr sgl::Vec2f scorePos{24.f, 14.f};

} // namespace sgl::tappy
