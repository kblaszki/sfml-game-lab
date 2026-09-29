#include <algorithm>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <sgl/ui/RenderMenu.hpp>

namespace sgl
{
namespace
{

constexpr float kPanelPad{24.f};

[[nodiscard]] Rect<float> scaledAboutCenter(Rect<float> rect, float scale)
{
    if(scale == 1.f)
    {
        return rect;
    }
    const float cx = rect.pos.x + rect.size.x * 0.5f;
    const float cy = rect.pos.y + rect.size.y * 0.5f;
    const float w = rect.size.x * scale;
    const float h = rect.size.y * scale;
    return Rect<float>{.pos = {cx - w * 0.5f, cy - h * 0.5f}, .size = {w, h}};
}

[[nodiscard]] bool allScalesIdle(const HoverAnim& hover, std::size_t count)
{
    for(std::size_t i = 0; i < count; ++i)
    {
        if(hover.scale(i) != 1.f)
        {
            return false;
        }
    }
    return true;
}

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

void renderMenu(RenderQueue& queue, const MenuModel& model, const MenuStyle& style, Layer layer, const HoverAnim& hover)
{
    const auto buttons = model.buttons();
    if(style.cornerRadius <= 0.f && allScalesIdle(hover, buttons.size()))
    {
        renderMenu(queue, model, style, layer);
        return;
    }

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

    const Rect<float> panelRect{.pos = {minX - kPanelPad, minY - kPanelPad},
                                .size = {maxX - minX + kPanelPad * 2.f, maxY - minY + kPanelPad * 2.f}};

    float depth = 0.f;
    if(style.cornerRadius > 0.f)
    {
        queue.push(layer, depth++, roundedRect(panelRect, style.cornerRadius, style.panel));
    }
    else
    {
        queue.push(layer,
                   depth++,
                   RectCmd{
                       .rect = panelRect,
                       .fill = style.panel,
                   });
    }

    const std::size_t focused = model.focus();
    for(std::size_t i = 0; i < buttons.size(); ++i)
    {
        const Button& button = buttons[i];
        const Rect<float> drawn = scaledAboutCenter(button.rect, hover.scale(i));
        const Color fill = (i == focused) ? style.focused : style.idle;

        if(style.cornerRadius > 0.f)
        {
            queue.push(layer, depth++, roundedRect(drawn, style.cornerRadius, fill));
        }
        else
        {
            queue.push(layer,
                       depth++,
                       RectCmd{
                           .rect = drawn,
                           .fill = fill,
                       });
        }

        queue.push(layer,
                   depth++,
                   TextCmd{
                       .font = style.font,
                       .text = button.label,
                       .size = style.textSize,
                       .position = {drawn.pos.x + drawn.size.x * 0.5f, drawn.pos.y + drawn.size.y * 0.5f},
                       .color = style.text,
                       .anchor = Anchor::Center,
                   });
    }
}

} // namespace sgl
