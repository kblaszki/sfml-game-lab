#include <syndicate/app/InsiderPeekBanner.hpp>

#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <sstream>
#include <string>
#include <syndicate/app/Describe.hpp>

namespace sgl::syndicate
{
namespace
{

constexpr sgl::Rect<float> kBanner{.pos = {400.f, 48.f}, .size = {480.f, 56.f}};

} // namespace

bool InsiderPeekBanner::visible(const GameState& state) const noexcept
{
    return state.revealed.has_value();
}

void InsiderPeekBanner::render(sgl::RenderQueue& queue, const GameState& state, sgl::FontId font, const Theme& theme) const
{
    if(!state.revealed.has_value())
    {
        return;
    }
    const MarketEventCard& peek = *state.revealed;
    const sgl::Color sectorColor = theme.sector[static_cast<std::size_t>(peek.sector)];

    float depth = 90.f;
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(kBanner, 8.f, sgl::Color{28, 22, 48, 235}));
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRectOutline(kBanner, 8.f, 2.f, sectorColor));

    std::ostringstream body;
    body << "Insider peek: " << sectorName(peek.sector) << ' ';
    if(peek.delta >= 0)
    {
        body << '+';
    }
    body << peek.delta;

    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font,
                   .text = body.str(),
                   .size = 18,
                   .position = {kBanner.pos.x + 20.f, kBanner.pos.y + 16.f},
                   .color = sgl::Color{245, 235, 255},
               });
}

} // namespace sgl::syndicate
