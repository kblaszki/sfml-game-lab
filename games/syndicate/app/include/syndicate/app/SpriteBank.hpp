#pragma once

#include <optional>
#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/resources/Atlas.hpp>
#include <string_view>

namespace sgl::syndicate
{

class SpriteBank
{
public:
    SpriteBank() = default;
    SpriteBank(sgl::TextureId texture, sgl::AtlasManifest manifest);

    [[nodiscard]] std::optional<sgl::SpriteCmd> sprite(std::string_view name,
                                                       sgl::Vec2f pos,
                                                       sgl::Vec2f size,
                                                       sgl::Color tint) const;

private:
    sgl::TextureId texture_{};
    sgl::AtlasManifest manifest_{};
};

} // namespace sgl::syndicate
