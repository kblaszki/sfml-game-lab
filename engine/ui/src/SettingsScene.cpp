#include <sgl/ui/SettingsScene.hpp>

#include <algorithm>
#include <sgl/core/Color.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/render/Shapes.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <utility>

namespace sgl
{
namespace
{

constexpr float kOverlayWidth{1280.f};
constexpr float kOverlayHeight{720.f};
constexpr float kPanelWidth{560.f};
constexpr float kPanelHeight{320.f};
constexpr float kPanelX{(kOverlayWidth - kPanelWidth) * 0.5f};
constexpr float kPanelY{(kOverlayHeight - kPanelHeight) * 0.5f};
constexpr float kRowHeight{52.f};
constexpr float kRowGap{12.f};
constexpr float kRowStartY{kPanelY + 36.f};
constexpr float kRowX{kPanelX + 28.f};
constexpr float kRowWidth{kPanelWidth - 56.f};
constexpr float kChevronWidth{44.f};
constexpr std::size_t kRowCount{4};

[[nodiscard]] std::size_t resolutionIndex(Vec2u resolution) noexcept
{
    for(std::size_t i = 0; i < kResolutions.size(); ++i)
    {
        if(kResolutions[i] == resolution)
        {
            return i;
        }
    }
    return 0;
}

} // namespace

SettingsScene::SettingsScene(Settings initial,
                             MenuActions actions,
                             FontId font,
                             std::function<void(const Settings&)> onChange)
    : settings_{initial}
    , actions_{actions}
    , font_{font}
    , onChange_{std::move(onChange)}
{
    if(!onChange_)
    {
        onChange_ = [](const Settings&) {};
    }
}

void SettingsScene::applyChange()
{
    onChange_(settings_);
}

void SettingsScene::nudge(int direction)
{
    if(direction == 0)
    {
        return;
    }

    const auto row = static_cast<Row>(focus_);
    switch(row)
    {
    case Row::Volume:
    {
        const int next = std::clamp(settings_.masterVolume + direction * 10, 0, 100);
        if(next == settings_.masterVolume)
        {
            return;
        }
        settings_.masterVolume = next;
        applyChange();
        break;
    }
    case Row::Window:
    {
        settings_.mode =
            settings_.mode == WindowMode::Windowed ? WindowMode::Fullscreen : WindowMode::Windowed;
        applyChange();
        break;
    }
    case Row::Resolution:
    {
        const std::size_t idx = resolutionIndex(settings_.resolution);
        const std::size_t count = kResolutions.size();
        const std::size_t next = static_cast<std::size_t>((static_cast<int>(idx) + direction + static_cast<int>(count))
                                                          % static_cast<int>(count));
        if(kResolutions[next] == settings_.resolution)
        {
            return;
        }
        settings_.resolution = kResolutions[next];
        applyChange();
        break;
    }
    case Row::Back:
    case Row::Count:
        break;
    }
}

std::string SettingsScene::rowLabel(Row row) const
{
    switch(row)
    {
    case Row::Volume:
        return "Master volume: " + std::to_string(settings_.masterVolume) + "%";
    case Row::Window:
        return settings_.mode == WindowMode::Fullscreen ? "Window: Fullscreen" : "Window: Windowed";
    case Row::Resolution:
        return "Resolution: " + std::to_string(settings_.resolution.x) + "x"
               + std::to_string(settings_.resolution.y);
    case Row::Back:
        return "Back";
    case Row::Count:
        break;
    }
    return {};
}

Rect<float> SettingsScene::rowRect(std::size_t index) const noexcept
{
    return Rect<float>{
        .pos = {kRowX, kRowStartY + static_cast<float>(index) * (kRowHeight + kRowGap)},
        .size = {kRowWidth, kRowHeight},
    };
}

Rect<float> SettingsScene::leftButtonRect(std::size_t index) const noexcept
{
    const Rect<float> row = rowRect(index);
    return Rect<float>{
        .pos = {row.pos.x + 8.f, row.pos.y + 6.f},
        .size = {kChevronWidth, row.size.y - 12.f},
    };
}

Rect<float> SettingsScene::rightButtonRect(std::size_t index) const noexcept
{
    const Rect<float> row = rowRect(index);
    return Rect<float>{
        .pos = {row.pos.x + row.size.x - kChevronWidth - 8.f, row.pos.y + 6.f},
        .size = {kChevronWidth, row.size.y - 12.f},
    };
}

std::optional<std::size_t> SettingsScene::hitRow(Vec2f point) const noexcept
{
    for(std::size_t i = 0; i < kRowCount; ++i)
    {
        if(rowRect(i).contains(point))
        {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<int> SettingsScene::hitChevron(Vec2f point) const noexcept
{
    if(focus_ >= static_cast<std::size_t>(Row::Back))
    {
        return std::nullopt;
    }
    if(leftButtonRect(focus_).contains(point))
    {
        return -1;
    }
    if(rightButtonRect(focus_).contains(point))
    {
        return 1;
    }
    return std::nullopt;
}

void SettingsScene::update(SceneContext& ctx, Seconds dt)
{
    const InputState& input = ctx.input();

    if(input.action(actions_.cancel).pressed)
    {
        ctx.request(PopScene{});
        return;
    }

    if(input.action(actions_.up).pressed)
    {
        focus_ = (focus_ + kRowCount - 1) % kRowCount;
    }
    if(input.action(actions_.down).pressed)
    {
        focus_ = (focus_ + 1) % kRowCount;
    }

    if(const auto pointer = input.pointer())
    {
        if(const auto hit = hitRow(*pointer))
        {
            focus_ = *hit;
        }
    }

    std::optional<std::size_t> hovered;
    if(const auto pointer = input.pointer())
    {
        hovered = hitRow(*pointer);
    }
    hover_.update(hovered, kRowCount, dt);

    if(focus_ < static_cast<std::size_t>(Row::Back))
    {
        if(input.action(actions_.left).pressed)
        {
            nudge(-1);
        }
        if(input.action(actions_.right).pressed)
        {
            nudge(1);
        }
    }

    if(input.pointerReleased())
    {
        if(const auto pointer = input.pointer())
        {
            if(const auto dir = hitChevron(*pointer))
            {
                nudge(*dir);
                return;
            }
            if(focus_ == static_cast<std::size_t>(Row::Back) && rowRect(focus_).contains(*pointer))
            {
                ctx.request(PopScene{});
                return;
            }
        }
    }

    if(input.action(actions_.confirm).pressed && focus_ == static_cast<std::size_t>(Row::Back))
    {
        ctx.request(PopScene{});
    }
}

void SettingsScene::render(RenderQueue& queue) const
{
    queue.push(Layer::Overlay,
               0.f,
               RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {kOverlayWidth, kOverlayHeight}},
                   .fill = Color{0, 0, 0, 150},
               });

    const Rect<float> panel{.pos = {kPanelX, kPanelY}, .size = {kPanelWidth, kPanelHeight}};
    queue.push(Layer::Overlay, 1.f, roundedRect(panel, 12.f, Color{24, 28, 48, 230}));

    constexpr MenuStyle style{
        .font = {},
        .textSize = 22,
        .idle = Color{48, 56, 88, 230},
        .focused = Color{72, 96, 160, 255},
        .text = Color{255, 255, 255},
        .panel = Color{24, 28, 48, 230},
        .cornerRadius = 8.f,
        .bodyFont = {},
    };

    float depth = 2.f;
    for(std::size_t i = 0; i < kRowCount; ++i)
    {
        const Rect<float> row = rowRect(i);
        const float scale = hover_.scale(i);
        Rect<float> drawn = row;
        if(scale != 1.f)
        {
            const float cx = row.pos.x + row.size.x * 0.5f;
            const float cy = row.pos.y + row.size.y * 0.5f;
            drawn.size = {row.size.x * scale, row.size.y * scale};
            drawn.pos = {cx - drawn.size.x * 0.5f, cy - drawn.size.y * 0.5f};
        }

        const Color fill = (i == focus_) ? style.focused : style.idle;
        queue.push(Layer::Overlay, depth++, roundedRect(drawn, style.cornerRadius, fill));

        const auto rowKind = static_cast<Row>(i);
        const std::string label = rowLabel(rowKind);
        queue.push(Layer::Overlay,
                   depth++,
                   TextCmd{
                       .font = font_,
                       .text = label,
                       .size = style.textSize,
                       .position = {drawn.pos.x + drawn.size.x * 0.5f, drawn.pos.y + drawn.size.y * 0.5f},
                       .color = style.text,
                       .anchor = Anchor::Center,
                   });

        if(rowKind != Row::Back)
        {
            const Rect<float> left = leftButtonRect(i);
            const Rect<float> right = rightButtonRect(i);
            queue.push(Layer::Overlay, depth++, roundedRect(left, 6.f, Color{36, 42, 64, 255}));
            queue.push(Layer::Overlay, depth++, roundedRect(right, 6.f, Color{36, 42, 64, 255}));
            queue.push(Layer::Overlay,
                       depth++,
                       TextCmd{
                           .font = font_,
                           .text = "<",
                           .size = 20,
                           .position = {left.pos.x + left.size.x * 0.5f, left.pos.y + left.size.y * 0.5f},
                           .color = style.text,
                           .anchor = Anchor::Center,
                       });
            queue.push(Layer::Overlay,
                       depth++,
                       TextCmd{
                           .font = font_,
                           .text = ">",
                           .size = 20,
                           .position = {right.pos.x + right.size.x * 0.5f, right.pos.y + right.size.y * 0.5f},
                           .color = style.text,
                           .anchor = Anchor::Center,
                       });
        }
    }
}

SceneTraits SettingsScene::traits() const
{
    return SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl
