#include <sgl/render/Shapes.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace sgl
{
namespace
{

constexpr float kEps = 1e-3f;

void appendTri(MeshCmd& mesh, Vec2f a, Vec2f b, Vec2f c, Color color)
{
    mesh.vertices.push_back(Vertex{.position = a, .color = color});
    mesh.vertices.push_back(Vertex{.position = b, .color = color});
    mesh.vertices.push_back(Vertex{.position = c, .color = color});
}

void appendQuad(MeshCmd& mesh, Vec2f a, Vec2f b, Vec2f c, Vec2f d, Color color)
{
    appendTri(mesh, a, b, c, color);
    appendTri(mesh, a, c, d, color);
}

[[nodiscard]] MeshCmd flatRect(Rect<float> rect, Color color)
{
    MeshCmd mesh{};
    const Vec2f tl = rect.pos;
    const Vec2f tr{rect.pos.x + rect.size.x, rect.pos.y};
    const Vec2f br{rect.pos.x + rect.size.x, rect.pos.y + rect.size.y};
    const Vec2f bl{rect.pos.x, rect.pos.y + rect.size.y};
    appendQuad(mesh, tl, tr, br, bl, color);
    return mesh;
}

void appendQuarter(MeshCmd& mesh, Vec2f center, float radius, float angleStart, int segments, Color color)
{
    const float step = (std::numbers::pi_v<float> * 0.5f) / static_cast<float>(segments);
    for(int i = 0; i < segments; ++i)
    {
        const float a0 = angleStart + step * static_cast<float>(i);
        const float a1 = angleStart + step * static_cast<float>(i + 1);
        const Vec2f p0{center.x + radius * std::cos(a0), center.y + radius * std::sin(a0)};
        const Vec2f p1{center.x + radius * std::cos(a1), center.y + radius * std::sin(a1)};
        appendTri(mesh, center, p0, p1, color);
    }
}

// Closed perimeter (first point repeated at end). Always uses 4 arcs of
// segments+1 samples so outer/inner rings share topology even when r == 0.
void appendRoundedPerimeter(std::vector<Vec2f>& out, Rect<float> rect, float radius, int segments)
{
    const float x = rect.pos.x;
    const float y = rect.pos.y;
    const float w = rect.size.x;
    const float h = rect.size.y;
    const float r = std::max(0.f, radius);

    const Vec2f tl{x + r, y + r};
    const Vec2f tr{x + w - r, y + r};
    const Vec2f br{x + w - r, y + h - r};
    const Vec2f bl{x + r, y + h - r};

    const auto appendArc = [&](Vec2f center, float angleStart) {
        const float step = (std::numbers::pi_v<float> * 0.5f) / static_cast<float>(segments);
        for(int i = 0; i <= segments; ++i)
        {
            const float a = angleStart + step * static_cast<float>(i);
            out.push_back(Vec2f{center.x + r * std::cos(a), center.y + r * std::sin(a)});
        }
    };

    // Top-right, bottom-right, bottom-left, top-left (CW with y-down).
    appendArc(tr, -std::numbers::pi_v<float> * 0.5f);
    appendArc(br, 0.f);
    appendArc(bl, std::numbers::pi_v<float> * 0.5f);
    appendArc(tl, std::numbers::pi_v<float>);
    out.push_back(out.front());
}

} // namespace

MeshCmd roundedRect(Rect<float> rect, float radius, Color color, int segments)
{
    const float w = rect.size.x;
    const float h = rect.size.y;
    if(w <= 0.f || h <= 0.f)
    {
        return flatRect(rect, color);
    }

    const float maxR = std::min(w, h) * 0.5f;
    const float r = std::clamp(radius, 0.f, maxR);
    if(r <= 0.f)
    {
        return flatRect(rect, color);
    }

    if(segments < 1)
    {
        segments = 1;
    }

    MeshCmd mesh{};
    const float x = rect.pos.x;
    const float y = rect.pos.y;

    // Full-height body inset by radius on X.
    appendQuad(mesh,
               {x + r, y},
               {x + w - r, y},
               {x + w - r, y + h},
               {x + r, y + h},
               color);

    // Side strips between the corner centers (left / right).
    appendQuad(mesh,
               {x, y + r},
               {x + r, y + r},
               {x + r, y + h - r},
               {x, y + h - r},
               color);
    appendQuad(mesh,
               {x + w - r, y + r},
               {x + w, y + r},
               {x + w, y + h - r},
               {x + w - r, y + h - r},
               color);

    // Quarter-circles (y-down angles): TL, TR, BR, BL.
    appendQuarter(mesh, {x + r, y + r}, r, std::numbers::pi_v<float>, segments, color);
    appendQuarter(mesh, {x + w - r, y + r}, r, -std::numbers::pi_v<float> * 0.5f, segments, color);
    appendQuarter(mesh, {x + w - r, y + h - r}, r, 0.f, segments, color);
    appendQuarter(mesh, {x + r, y + h - r}, r, std::numbers::pi_v<float> * 0.5f, segments, color);

    return mesh;
}

MeshCmd roundedRectOutline(Rect<float> rect, float radius, float thickness, Color color, int segments)
{
    MeshCmd mesh{};
    const float w = rect.size.x;
    const float h = rect.size.y;
    if(thickness <= 0.f || w <= 0.f || h <= 0.f)
    {
        return mesh;
    }
    if(thickness * 2.f >= w + kEps || thickness * 2.f >= h + kEps)
    {
        return mesh;
    }

    const float maxR = std::min(w, h) * 0.5f;
    const float outerR = std::clamp(radius, 0.f, maxR);
    if(segments < 1)
    {
        segments = 1;
    }

    const Rect<float> inner{.pos = {rect.pos.x + thickness, rect.pos.y + thickness},
                            .size = {w - 2.f * thickness, h - 2.f * thickness}};
    if(inner.size.x <= 0.f || inner.size.y <= 0.f)
    {
        return mesh;
    }

    const float innerMaxR = std::min(inner.size.x, inner.size.y) * 0.5f;
    const float innerR = std::clamp(outerR - thickness, 0.f, innerMaxR);

    std::vector<Vec2f> outer;
    std::vector<Vec2f> innerPts;
    appendRoundedPerimeter(outer, rect, outerR, segments);
    appendRoundedPerimeter(innerPts, inner, innerR, segments);
    if(outer.size() != innerPts.size() || outer.size() < 2)
    {
        return MeshCmd{};
    }

    // Perimeter includes a repeated closing point; quads along consecutive pairs.
    for(std::size_t i = 0; i + 1 < outer.size(); ++i)
    {
        appendQuad(mesh, outer[i], outer[i + 1], innerPts[i + 1], innerPts[i], color);
    }
    return mesh;
}

MeshCmd verticalGradient(Rect<float> rect, Color top, Color bottom)
{
    MeshCmd mesh{};
    const Vec2f tl = rect.pos;
    const Vec2f tr{rect.pos.x + rect.size.x, rect.pos.y};
    const Vec2f br{rect.pos.x + rect.size.x, rect.pos.y + rect.size.y};
    const Vec2f bl{rect.pos.x, rect.pos.y + rect.size.y};

    mesh.vertices.push_back(Vertex{.position = tl, .color = top});
    mesh.vertices.push_back(Vertex{.position = tr, .color = top});
    mesh.vertices.push_back(Vertex{.position = br, .color = bottom});

    mesh.vertices.push_back(Vertex{.position = tl, .color = top});
    mesh.vertices.push_back(Vertex{.position = br, .color = bottom});
    mesh.vertices.push_back(Vertex{.position = bl, .color = bottom});
    return mesh;
}

MeshCmd radialGlow(Vec2f center, float innerRadius, float outerRadius, Color inner, int segments)
{
    MeshCmd mesh{};
    if(outerRadius <= innerRadius || segments < 3)
    {
        return mesh;
    }

    const Color outerColor{inner.r, inner.g, inner.b, 0};
    const float twoPi = std::numbers::pi_v<float> * 2.f;
    for(int i = 0; i < segments; ++i)
    {
        const float a0 = twoPi * static_cast<float>(i) / static_cast<float>(segments);
        const float a1 = twoPi * static_cast<float>(i + 1) / static_cast<float>(segments);
        const Vec2f in0{center.x + innerRadius * std::cos(a0), center.y + innerRadius * std::sin(a0)};
        const Vec2f out0{center.x + outerRadius * std::cos(a0), center.y + outerRadius * std::sin(a0)};
        const Vec2f out1{center.x + outerRadius * std::cos(a1), center.y + outerRadius * std::sin(a1)};
        mesh.vertices.push_back(Vertex{.position = in0, .color = inner});
        mesh.vertices.push_back(Vertex{.position = out0, .color = outerColor});
        mesh.vertices.push_back(Vertex{.position = out1, .color = outerColor});
    }
    return mesh;
}

MeshCmd polyline(std::span<const Vec2f> points, float thickness, Color color)
{
    MeshCmd mesh{};
    if(points.size() < 2 || !(thickness > 0.f))
    {
        return mesh;
    }

    const float half = thickness * 0.5f;
    for(std::size_t i = 0; i + 1 < points.size(); ++i)
    {
        const Vec2f a = points[i];
        const Vec2f b = points[i + 1];
        const Vec2f delta = b - a;
        const float lenSq = lengthSq(delta);
        if(!(lenSq > 0.f))
        {
            continue;
        }
        const float invLen = 1.f / std::sqrt(lenSq);
        const Vec2f dir{delta.x * invLen, delta.y * invLen};
        const Vec2f perp{-dir.y * half, dir.x * half};
        appendQuad(mesh, a + perp, a - perp, b - perp, b + perp, color);
    }
    return mesh;
}

MeshCmd arrowHead(Vec2f tip, Vec2f direction, float size, Color color)
{
    MeshCmd mesh{};
    const Vec2f dir = normalized(direction).value_or(Vec2f{1.f, 0.f});
    const Vec2f perp{-dir.y, dir.x};
    const Vec2f base = tip - dir * size;
    const Vec2f left = base + perp * (size * 0.5f);
    const Vec2f right = base - perp * (size * 0.5f);
    mesh.vertices.push_back(Vertex{.position = tip, .color = color});
    mesh.vertices.push_back(Vertex{.position = left, .color = color});
    mesh.vertices.push_back(Vertex{.position = right, .color = color});
    return mesh;
}

} // namespace sgl
