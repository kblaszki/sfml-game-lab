#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/AssetIds.hpp>
#include <string>
#include <variant>
#include <vector>

namespace sgl
{

enum class Layer : std::uint8_t
{
    Background,
    World,
    Actors,
    Hud,
    Overlay
};

struct SortKey
{
    Layer layer{};
    float depth{};
    std::uint32_t seq{};

    auto operator<=>(const SortKey&) const = default;
};

struct SpriteCmd
{
    TextureId texture;
    Rect<float> source;
    Vec2f position;
    Vec2f scale{1.f, 1.f};
    Color tint{255, 255, 255, 255};
    float rotationDeg{};
};

struct RectCmd
{
    Rect<float> rect;
    Color fill;
    Color outline{0, 0, 0, 0};
    float outlineThickness{};
    float rotationDeg{};
};

enum class Anchor
{
    TopLeft,
    Center
};

struct TextCmd
{
    FontId font;
    std::string text;
    unsigned size;
    Vec2f position;
    Color color;
    Anchor anchor{Anchor::TopLeft};
};

struct LineCmd
{
    Vec2f from;
    Vec2f to;
    float thickness{1.f};
    Color color;
};

struct CircleCmd
{
    Vec2f center;
    float radius;
    Color fill;
    Color outline{};
    float outlineThickness{};
    std::size_t points{32};
};

struct Vertex
{
    Vec2f position;
    Color color;
    Vec2f uv{};
};

enum class MeshKind : std::uint8_t
{
    Triangles,
    TriangleStrip
};

struct MeshCmd
{
    std::vector<Vertex> vertices;
    MeshKind kind{MeshKind::Triangles};
    std::optional<TextureId> texture{};
};

using DrawCommand = std::variant<SpriteCmd, RectCmd, TextCmd, LineCmd, CircleCmd, MeshCmd>;

struct Entry
{
    SortKey key;
    DrawCommand command;
};

} // namespace sgl
