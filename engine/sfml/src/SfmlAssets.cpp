#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

#include <cstdint>
#include <sgl/sfml/SfmlAssets.hpp>

namespace sgl::sfml
{
namespace
{

sf::Color toSfColor(sgl::Color color)
{
    return sf::Color{color.r, color.g, color.b, color.a};
}

std::expected<sf::Texture, sgl::ResourceError> loadTexture(const sgl::Image& image)
{
    if(image.width == 0 || image.height == 0
       || image.pixels.size() != static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height))
    {
        return std::unexpected(sgl::ResourceError::DecodeFailed);
    }

    sf::Image sfImage{sf::Vector2u{image.width, image.height}, sf::Color::Transparent};
    for(std::uint32_t y = 0; y < image.height; ++y)
    {
        for(std::uint32_t x = 0; x < image.width; ++x)
        {
            const sgl::Color pixel = image.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width)
                                                  + static_cast<std::size_t>(x)];
            sfImage.setPixel({x, y}, toSfColor(pixel));
        }
    }

    sf::Texture texture;
    if(!texture.loadFromImage(sfImage))
    {
        return std::unexpected(sgl::ResourceError::DecodeFailed);
    }
    return texture;
}

std::expected<sf::Font, sgl::ResourceError> loadFont(const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path))
    {
        return std::unexpected(sgl::ResourceError::NotFound);
    }

    sf::Font font;
    if(!font.openFromFile(path))
    {
        return std::unexpected(sgl::ResourceError::DecodeFailed);
    }
    return font;
}

} // namespace

std::expected<sgl::TextureId, sgl::ResourceError> SfmlAssets::texture(std::string_view key, const sgl::Image& image)
{
    return textures_.load(key, [&]() { return loadTexture(image); });
}

std::optional<sgl::TextureId> SfmlAssets::textureFile(std::string_view key,
                                                      const std::filesystem::path& path,
                                                      bool smooth)
{
    const auto loaded = textures_.load(key, [&]() -> std::expected<sf::Texture, sgl::ResourceError> {
        sf::Texture texture;
        if(!texture.loadFromFile(path))
        {
            return std::unexpected(sgl::ResourceError::DecodeFailed);
        }
        texture.setSmooth(smooth);
        return texture;
    });
    if(!loaded)
    {
        return std::nullopt;
    }
    return *loaded;
}

std::expected<sgl::FontId, sgl::ResourceError> SfmlAssets::font(std::string_view key, const std::filesystem::path& path)
{
    return fonts_.load(key, [&]() { return loadFont(path); });
}

const sf::Texture* SfmlAssets::get(sgl::TextureId id) const
{
    return textures_.get(id);
}

const sf::Font* SfmlAssets::get(sgl::FontId id) const
{
    return fonts_.get(id);
}

} // namespace sgl::sfml
