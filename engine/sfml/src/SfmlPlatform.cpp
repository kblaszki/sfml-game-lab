#include <SFML/Window/ContextSettings.hpp>

#include <sgl/sfml/EventTranslate.hpp>
#include <sgl/sfml/SfmlAudio.hpp>
#include <sgl/sfml/SfmlPlatform.hpp>
#include <utility>

namespace sgl::sfml
{

SfmlPlatform::SfmlPlatform(sgl::Vec2u designSize, std::string title)
    : window_{}
    , assets_{}
    , audio_{std::make_unique<SfmlAudio>()}
    , renderer_{window_, assets_}
    , designSize_{designSize}
    , view_{sf::FloatRect{{0.f, 0.f}, {static_cast<float>(designSize.x), static_cast<float>(designSize.y)}}}
    , title_{std::move(title)}
{
    openDisplay(sgl::WindowMode::Windowed, designSize);
}

SfmlPlatform::~SfmlPlatform() = default;

void SfmlPlatform::setDisplay(sgl::WindowMode mode, sgl::Vec2u size)
{
    openDisplay(mode, size);
}

void SfmlPlatform::openDisplay(sgl::WindowMode mode, sgl::Vec2u size)
{
    const sf::ContextSettings settings{.antiAliasingLevel = 4};
    if(mode == sgl::WindowMode::Fullscreen)
    {
        window_.create(sf::VideoMode::getDesktopMode(), title_, sf::Style::Default, sf::State::Fullscreen, settings);
    }
    else
    {
        window_.create(sf::VideoMode{{size.x, size.y}}, title_, sf::Style::Default, sf::State::Windowed, settings);
    }

    window_.setKeyRepeatEnabled(false);
    window_.setFramerateLimit(60);
    view_ = sf::View{sf::FloatRect{{0.f, 0.f}, {static_cast<float>(designSize_.x), static_cast<float>(designSize_.y)}}};
    applyLetterbox(window_.getSize());
}

bool SfmlPlatform::isOpen() const
{
    return window_.isOpen();
}

void SfmlPlatform::close()
{
    window_.close();
}

std::optional<sgl::InputEvent> SfmlPlatform::poll()
{
    while(const std::optional<sf::Event> event = window_.pollEvent())
    {
        if(const auto* resized = event->getIf<sf::Event::Resized>())
        {
            applyLetterbox(resized->size);
            continue;
        }

        sf::Vector2f designPosition{};
        if(const std::optional<sf::Vector2i> pixel = pointerPixel(*event))
        {
            designPosition = window_.mapPixelToCoords(*pixel);
        }

        if(std::optional<sgl::InputEvent> input = translate(*event, designPosition))
        {
            return input;
        }
    }
    return std::nullopt;
}

sgl::RendererI& SfmlPlatform::renderer()
{
    return renderer_;
}

SfmlAssets& SfmlPlatform::assets()
{
    return assets_;
}

const SfmlAssets& SfmlPlatform::assets() const
{
    return assets_;
}

SfmlAudio& SfmlPlatform::audio()
{
    return *audio_;
}

const SfmlAudio& SfmlPlatform::audio() const
{
    return *audio_;
}

void SfmlPlatform::applyLetterbox(sf::Vector2u windowSize)
{
    if(windowSize.x == 0 || windowSize.y == 0 || designSize_.x == 0 || designSize_.y == 0)
    {
        return;
    }

    const float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    const float designRatio = static_cast<float>(designSize_.x) / static_cast<float>(designSize_.y);

    float viewportWidth = 1.f;
    float viewportHeight = 1.f;
    float viewportX = 0.f;
    float viewportY = 0.f;

    if(windowRatio > designRatio)
    {
        viewportWidth = designRatio / windowRatio;
        viewportX = (1.f - viewportWidth) * 0.5f;
    }
    else
    {
        viewportHeight = windowRatio / designRatio;
        viewportY = (1.f - viewportHeight) * 0.5f;
    }

    view_.setViewport(sf::FloatRect{{viewportX, viewportY}, {viewportWidth, viewportHeight}});
    window_.setView(view_);
}

} // namespace sgl::sfml
