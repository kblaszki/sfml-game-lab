#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Angle.hpp>

#include <cmath>
#include <sgl/render/DrawCommand.hpp>

namespace sgl::sfml
{
namespace detail
{

inline sf::Color toSfColor(sgl::Color color)
{
    return sf::Color{color.r, color.g, color.b, color.a};
}

} // namespace detail

// Position is the top-left of the unrotated bounds; rotation is around the center.
inline sf::RectangleShape toShape(const sgl::RectCmd& cmd)
{
    sf::RectangleShape shape{{cmd.rect.size.x, cmd.rect.size.y}};
    const float halfW = cmd.rect.size.x * 0.5f;
    const float halfH = cmd.rect.size.y * 0.5f;
    shape.setOrigin({halfW, halfH});
    shape.setPosition({cmd.rect.pos.x + halfW, cmd.rect.pos.y + halfH});
    shape.setRotation(sf::degrees(cmd.rotationDeg));
    shape.setFillColor(detail::toSfColor(cmd.fill));
    if(cmd.outlineThickness != 0.f)
    {
        shape.setOutlineThickness(cmd.outlineThickness);
        shape.setOutlineColor(detail::toSfColor(cmd.outline));
    }
    return shape;
}

// Length is |to-from|, height is thickness; origin at left-middle so position is `from`.
inline sf::RectangleShape toShape(const sgl::LineCmd& cmd)
{
    const float dx = cmd.to.x - cmd.from.x;
    const float dy = cmd.to.y - cmd.from.y;
    const float length = std::sqrt(dx * dx + dy * dy);
    sf::RectangleShape shape{{length, cmd.thickness}};
    shape.setOrigin({0.f, cmd.thickness * 0.5f});
    shape.setPosition({cmd.from.x, cmd.from.y});
    shape.setRotation(sf::radians(std::atan2(dy, dx)));
    shape.setFillColor(detail::toSfColor(cmd.color));
    return shape;
}

inline sf::CircleShape toShape(const sgl::CircleCmd& cmd)
{
    sf::CircleShape shape{cmd.radius, static_cast<std::size_t>(cmd.points)};
    shape.setOrigin({cmd.radius, cmd.radius});
    shape.setPosition({cmd.center.x, cmd.center.y});
    shape.setFillColor(detail::toSfColor(cmd.fill));
    if(cmd.outlineThickness != 0.f)
    {
        shape.setOutlineThickness(cmd.outlineThickness);
        shape.setOutlineColor(detail::toSfColor(cmd.outline));
    }
    return shape;
}

inline sf::Sprite toSprite(const sgl::SpriteCmd& cmd, const sf::Texture& texture)
{
    sf::Sprite sprite{texture};
    const auto left = static_cast<int>(std::lround(cmd.source.pos.x));
    const auto top = static_cast<int>(std::lround(cmd.source.pos.y));
    const auto width = static_cast<int>(std::lround(cmd.source.size.x));
    const auto height = static_cast<int>(std::lround(cmd.source.size.y));
    sprite.setTextureRect(sf::IntRect({left, top}, {width, height}));
    sprite.setScale({cmd.scale.x, cmd.scale.y});
    sprite.setColor(detail::toSfColor(cmd.tint));

    const float halfW = cmd.source.size.x * 0.5f;
    const float halfH = cmd.source.size.y * 0.5f;
    sprite.setOrigin({halfW, halfH});
    sprite.setPosition({cmd.position.x + halfW * cmd.scale.x, cmd.position.y + halfH * cmd.scale.y});
    sprite.setRotation(sf::degrees(cmd.rotationDeg));
    return sprite;
}

} // namespace sgl::sfml
