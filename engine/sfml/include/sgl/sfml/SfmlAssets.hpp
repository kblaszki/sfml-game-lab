#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <sgl/core/Image.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/resources/ResourceCache.hpp>
#include <sgl/resources/ResourceError.hpp>
#include <string_view>

namespace sgl::sfml
{

class SfmlAssets
{
public:
    [[nodiscard]] std::expected<sgl::TextureId, sgl::ResourceError> texture(std::string_view key,
                                                                            const sgl::Image& image);
    [[nodiscard]] std::optional<sgl::TextureId> textureFile(std::string_view key,
                                                            const std::filesystem::path& path,
                                                            bool smooth);
    [[nodiscard]] std::expected<sgl::FontId, sgl::ResourceError> font(std::string_view key,
                                                                      const std::filesystem::path& path);

    [[nodiscard]] const sf::Texture* get(sgl::TextureId id) const;
    [[nodiscard]] const sf::Font* get(sgl::FontId id) const;

private:
    sgl::ResourceCache<sgl::TextureId, sf::Texture> textures_{};
    sgl::ResourceCache<sgl::FontId, sf::Font> fonts_{};
};

} // namespace sgl::sfml
