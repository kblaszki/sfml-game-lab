#pragma once

#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <string_view>

namespace sgl::snake
{

inline constexpr float designWidth{1280.f};
inline constexpr float designHeight{720.f};
inline constexpr float cellSize{40.f};

inline constexpr std::string_view uiFontRelativePath{"fonts/upheavtt.ttf"};

inline constexpr sgl::Color titleGreen{170, 230, 120};
inline constexpr sgl::Color hudText{236, 245, 220};
inline constexpr sgl::Color gridFill{22, 36, 28};
inline constexpr sgl::Color gridFillAlt{28, 46, 34};
inline constexpr sgl::Color bodyFill{46, 160, 72};
inline constexpr sgl::Color headFill{190, 240, 90};
inline constexpr sgl::Color foodFill{220, 64, 54};
inline constexpr sgl::Color menuButton{36, 90, 48};
inline constexpr sgl::Color menuButtonFocus{64, 140, 72};
inline constexpr sgl::Color menuLabel{240, 255, 236};
inline constexpr sgl::Color overlayDim{0, 0, 0, 150};
inline constexpr sgl::Color panelBox{16, 28, 22, 230};
inline constexpr sgl::Color scorePlate{0, 0, 0, 150};

inline constexpr sgl::Vec2f scorePos{24.f, 14.f};

} // namespace sgl::snake
