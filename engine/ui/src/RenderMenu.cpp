#include <sgl/ui/RenderMenu.hpp>

#include <algorithm>
#include <sgl/render/DrawCommand.hpp>

namespace sgl
{
namespace
{

constexpr float kPanelPad{24.f};

} // namespace

void renderMenu(RenderQueue& queue, const MenuModel& model, const MenuStyle& style, Layer layer)
{
    const auto buttons = model.buttons();
    if(buttons.empty())
    {
        return;
    }

    float minX = buttons.front().rect.pos.x;
    float minY = buttons.front().rect.pos.y;
    float maxX = minX + buttons.front().rect.size.x;
    float maxY = minY + buttons.front().rect.size.y;
    for(const Button& button: buttons)
    {
        minX = std::min(minX, button.rect.pos.x);
        minY = std::min(minY, button.rect.pos.y);
        maxX = std::max(maxX, button.rect.pos.x + button.rect.size.x);
        maxY = std::max(maxY, button.rect.pos.y + button.rect.size.y);
    }

    float depth = 0.f;
    queue.push(layer,
               depth++,
               RectCmd{
                   .rect = {.pos = {minX - kPanelPad, minY - kPanelPad},
                            .size = {maxX - minX + kPanelPad * 2.f, maxY - minY + kPanelPad * 2.f}},
                   .fill = style.panel,
               });

    const std::size_t focused = model.focus();
    for(std::size_t i = 0; i < buttons.size(); ++i)
    {
        const Button& button = buttons[i];
        queue.push(layer,
                   depth++,
                   RectCmd{
                       .rect = button.rect,
                       .fill = (i == focused) ? style.focused : style.idle,
                   });
        queue.push(layer,
                   depth++,
                   TextCmd{
                       .font = style.font,
                       .text = button.label,
                       .size = style.textSize,
                       .position = {button.rect.pos.x + button.rect.size.x * 0.5f,
                                    button.rect.pos.y + button.rect.size.y * 0.5f},
                       .color = style.text,
                       .anchor = Anchor::Center,
                   });
    }
}

} // namespace sgl
