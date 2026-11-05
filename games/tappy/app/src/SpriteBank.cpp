#include <tappy/app/SpriteBank.hpp>
#include <utility>

namespace sgl::tappy
{

SpriteBank::SpriteBank(sgl::TextureId texture, sgl::AtlasManifest manifest)
    : texture_{texture}
    , manifest_{std::move(manifest)}
{
}

std::optional<sgl::SpriteCmd>
SpriteBank::sprite(std::string_view name, sgl::Vec2f pos, sgl::Vec2f size, sgl::Color tint, float rotationDeg) const
{
    const std::optional<sgl::AtlasRect> rect = manifest_.find(name);
    if(!rect)
    {
        return std::nullopt;
    }

    const sgl::Vec2f scale = (rect->w > 0 && rect->h > 0) ? sgl::Vec2f{size.x / static_cast<float>(rect->w),
                                                                       size.y / static_cast<float>(rect->h)}
                                                          : sgl::Vec2f{1.f, 1.f};

    return sgl::SpriteCmd{
        .texture = texture_,
        .source =
            sgl::Rect<float>{
                .pos = {static_cast<float>(rect->x), static_cast<float>(rect->y)},
                .size = {static_cast<float>(rect->w), static_cast<float>(rect->h)},
            },
        .position = pos,
        .scale = scale,
        .tint = tint,
        .rotationDeg = rotationDeg,
    };
}

} // namespace sgl::tappy
