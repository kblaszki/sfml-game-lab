#pragma once

#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <span>

namespace sgl
{

[[nodiscard]] MeshCmd roundedRect(Rect<float> rect, float radius, Color color, int segments = 6);
[[nodiscard]] MeshCmd roundedRectOutline(Rect<float> rect, float radius, float thickness, Color color, int segments = 6);
[[nodiscard]] MeshCmd verticalGradient(Rect<float> rect, Color top, Color bottom);
[[nodiscard]] MeshCmd radialGlow(Vec2f center, float innerRadius, float outerRadius, Color inner, int segments = 32);
[[nodiscard]] MeshCmd polyline(std::span<const Vec2f> points, float thickness, Color color);
[[nodiscard]] MeshCmd arrowHead(Vec2f tip, Vec2f direction, float size, Color color);

} // namespace sgl
