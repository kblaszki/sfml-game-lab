#pragma once

#include <SFML/Graphics/RenderTarget.hpp>

#include <cstdint>
#include <sgl/render/RendererI.hpp>
#include <sgl/sfml/SfmlAssets.hpp>

namespace sgl
{
class RenderQueue;
}

namespace sgl::sfml
{

class SfmlRenderer : public sgl::RendererI
{
public:
    SfmlRenderer(sf::RenderTarget& target, const SfmlAssets& assets);

    void begin() override;
    void submit(sgl::RenderQueue& queue) override;
    void end() override;

    [[nodiscard]] std::uint32_t skippedCommands() const;

private:
    sf::RenderTarget& target_;
    const SfmlAssets& assets_;
    std::uint32_t skipped_{};
};

} // namespace sgl::sfml
