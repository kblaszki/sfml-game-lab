#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include <memory>
#include <sgl/audio/AudioI.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/loop/PlatformI.hpp>
#include <sgl/sfml/SfmlAssets.hpp>
#include <sgl/sfml/SfmlRenderer.hpp>
#include <string>

namespace sgl::sfml
{

class SfmlAudio;

class SfmlPlatform : public sgl::PlatformI
{
public:
    SfmlPlatform(sgl::Vec2u designSize, std::string title);
    ~SfmlPlatform() override;

    SfmlPlatform(const SfmlPlatform&) = delete;
    SfmlPlatform& operator=(const SfmlPlatform&) = delete;

    bool isOpen() const override;
    void close() override;
    std::optional<sgl::InputEvent> poll() override;
    sgl::RendererI& renderer() override;

    [[nodiscard]] SfmlAssets& assets();
    [[nodiscard]] const SfmlAssets& assets() const;

    [[nodiscard]] SfmlAudio& audio();
    [[nodiscard]] const SfmlAudio& audio() const;

private:
    void applyLetterbox(sf::Vector2u windowSize);

    // window, assets, audio, then renderer — GL context and audio device outlive users.
    sf::RenderWindow window_;
    SfmlAssets assets_;
    std::unique_ptr<SfmlAudio> audio_;
    SfmlRenderer renderer_;
    sgl::Vec2u designSize_{};
    sf::View view_{};
};

} // namespace sgl::sfml
