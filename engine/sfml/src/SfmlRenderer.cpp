#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/sfml/SfmlDraw.hpp>
#include <sgl/sfml/SfmlRenderer.hpp>
#include <type_traits>
#include <variant>

namespace sgl::sfml
{
namespace
{

sf::Color toSfColor(sgl::Color color)
{
    return sf::Color{color.r, color.g, color.b, color.a};
}

void setCenterOrigin(sf::Transformable& transformable, const sf::FloatRect& bounds)
{
    transformable.setOrigin({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
}

} // namespace

SfmlRenderer::SfmlRenderer(sf::RenderTarget& target, const SfmlAssets& assets)
    : target_{target}
    , assets_{assets}
{
}

void SfmlRenderer::begin()
{
    skipped_ = 0;
    target_.clear();
}

void SfmlRenderer::submit(sgl::RenderQueue& queue)
{
    for(const sgl::Entry& entry: queue.sorted())
    {
        std::visit(
            [this](const auto& cmd) {
                using T = std::decay_t<decltype(cmd)>;
                if constexpr(std::is_same_v<T, sgl::SpriteCmd>)
                {
                    const sf::Texture* texture = assets_.get(cmd.texture);
                    if(texture == nullptr)
                    {
                        ++skipped_;
                        return;
                    }
                    target_.draw(toSprite(cmd, *texture));
                }
                else if constexpr(std::is_same_v<T, sgl::RectCmd>)
                {
                    target_.draw(toShape(cmd));
                }
                else if constexpr(std::is_same_v<T, sgl::TextCmd>)
                {
                    const sf::Font* font = assets_.get(cmd.font);
                    if(font == nullptr)
                    {
                        ++skipped_;
                        return;
                    }

                    sf::Text text{*font, cmd.text, cmd.size};
                    text.setFillColor(toSfColor(cmd.color));
                    if(cmd.anchor == sgl::Anchor::Center)
                    {
                        setCenterOrigin(text, text.getLocalBounds());
                    }
                    text.setPosition({cmd.position.x, cmd.position.y});
                    target_.draw(text);
                }
                else if constexpr(std::is_same_v<T, sgl::LineCmd> || std::is_same_v<T, sgl::CircleCmd>)
                {
                    target_.draw(toShape(cmd));
                }
                else if constexpr(std::is_same_v<T, sgl::MeshCmd>)
                {
                    const sf::VertexArray vertices = toVertexArray(cmd);
                    if(!cmd.texture.has_value())
                    {
                        target_.draw(vertices);
                        return;
                    }
                    const sf::Texture* texture = assets_.get(*cmd.texture);
                    if(texture == nullptr)
                    {
                        ++skipped_;
                        return;
                    }
                    sf::RenderStates states;
                    states.texture = texture;
                    target_.draw(vertices, states);
                }
            },
            entry.command);
    }
}

void SfmlRenderer::end()
{
    if(auto* window = dynamic_cast<sf::RenderWindow*>(&target_))
    {
        window->display();
    }
}

std::uint32_t SfmlRenderer::skippedCommands() const
{
    return skipped_;
}

} // namespace sgl::sfml
