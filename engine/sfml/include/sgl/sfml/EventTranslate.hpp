#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <optional>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/input/MouseButton.hpp>

namespace sgl::sfml
{

[[nodiscard]] constexpr sgl::Key toKey(sf::Keyboard::Key key) noexcept
{
    switch(key)
    {
        case sf::Keyboard::Key::A:
            return sgl::Key::A;
        case sf::Keyboard::Key::D:
            return sgl::Key::D;
        case sf::Keyboard::Key::W:
            return sgl::Key::W;
        case sf::Keyboard::Key::S:
            return sgl::Key::S;
        case sf::Keyboard::Key::Z:
            return sgl::Key::Z;
        case sf::Keyboard::Key::X:
            return sgl::Key::X;
        case sf::Keyboard::Key::C:
            return sgl::Key::C;
        case sf::Keyboard::Key::P:
            return sgl::Key::P;
        case sf::Keyboard::Key::LShift:
            return sgl::Key::LShift;
        case sf::Keyboard::Key::Left:
            return sgl::Key::Left;
        case sf::Keyboard::Key::Right:
            return sgl::Key::Right;
        case sf::Keyboard::Key::Up:
            return sgl::Key::Up;
        case sf::Keyboard::Key::Down:
            return sgl::Key::Down;
        case sf::Keyboard::Key::Enter:
            return sgl::Key::Enter;
        case sf::Keyboard::Key::Escape:
            return sgl::Key::Escape;
        case sf::Keyboard::Key::Backspace:
            return sgl::Key::Backspace;
        case sf::Keyboard::Key::Space:
            return sgl::Key::Space;
        default:
            return sgl::Key::Unknown;
    }
}

[[nodiscard]] inline std::optional<sgl::MouseButton> toMouseButton(sf::Mouse::Button button) noexcept
{
    switch(button)
    {
        case sf::Mouse::Button::Left:
            return sgl::MouseButton::Left;
        case sf::Mouse::Button::Right:
            return sgl::MouseButton::Right;
        case sf::Mouse::Button::Middle:
            return sgl::MouseButton::Middle;
        default:
            return std::nullopt;
    }
}

[[nodiscard]] inline std::optional<sf::Vector2i> pointerPixel(const sf::Event& event) noexcept
{
    if(const auto* moved = event.getIf<sf::Event::MouseMoved>())
    {
        return moved->position;
    }
    if(const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        return pressed->position;
    }
    if(const auto* released = event.getIf<sf::Event::MouseButtonReleased>())
    {
        return released->position;
    }
    return std::nullopt;
}

[[nodiscard]] inline std::optional<sgl::InputEvent> translate(const sf::Event& event, sf::Vector2f designPosition)
{
    if(event.is<sf::Event::Closed>())
    {
        return sgl::WindowClosed{};
    }
    if(event.is<sf::Event::FocusLost>())
    {
        return sgl::FocusLost{};
    }
    if(event.is<sf::Event::FocusGained>())
    {
        return sgl::FocusGained{};
    }
    if(const auto* pressed = event.getIf<sf::Event::KeyPressed>())
    {
        return sgl::KeyDown{.key = toKey(pressed->code)};
    }
    if(const auto* released = event.getIf<sf::Event::KeyReleased>())
    {
        return sgl::KeyUp{.key = toKey(released->code)};
    }
    if(const auto* mouseDown = event.getIf<sf::Event::MouseButtonPressed>())
    {
        const std::optional<sgl::MouseButton> button = toMouseButton(mouseDown->button);
        if(!button)
        {
            return std::nullopt;
        }
        return sgl::MouseDown{.button = *button, .pos = {designPosition.x, designPosition.y}};
    }
    if(const auto* mouseUp = event.getIf<sf::Event::MouseButtonReleased>())
    {
        const std::optional<sgl::MouseButton> button = toMouseButton(mouseUp->button);
        if(!button)
        {
            return std::nullopt;
        }
        return sgl::MouseUp{.button = *button, .pos = {designPosition.x, designPosition.y}};
    }
    if(event.is<sf::Event::MouseMoved>())
    {
        return sgl::MouseMove{.pos = {designPosition.x, designPosition.y}};
    }
    return std::nullopt;
}

} // namespace sgl::sfml
