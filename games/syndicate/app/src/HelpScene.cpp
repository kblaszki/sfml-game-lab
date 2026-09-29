#include <syndicate/app/HelpScene.hpp>

#include <algorithm>
#include <optional>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/render/Shapes.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <syndicate/app/ControlMap.hpp>

namespace sgl::syndicate
{
namespace
{

constexpr float kOverlayWidth{1280.f};
constexpr float kOverlayHeight{720.f};
constexpr float kPanelWidth{720.f};
constexpr float kPanelHeight{480.f};
constexpr float kPanelX{(kOverlayWidth - kPanelWidth) * 0.5f};
constexpr float kPanelY{(kOverlayHeight - kPanelHeight) * 0.5f};
constexpr float kTabHeight{36.f};
constexpr float kTabGap{8.f};
constexpr float kBackY{kPanelY + kPanelHeight - 64.f};

[[nodiscard]] std::vector<std::string_view> uniqueTabs()
{
    std::vector<std::string_view> tabs;
    for(const ControlRow& row: controlMap())
    {
        if(tabs.empty() || tabs.back() != row.phase)
        {
            if(std::find(tabs.begin(), tabs.end(), row.phase) == tabs.end())
            {
                tabs.push_back(row.phase);
            }
        }
    }
    return tabs;
}

[[nodiscard]] sgl::Rect<float> tabRect(std::size_t index, std::size_t count)
{
    const float totalGap = kTabGap * static_cast<float>(count > 0 ? count - 1 : 0);
    const float width = (kPanelWidth - 48.f - totalGap) / static_cast<float>(std::max<std::size_t>(count, 1));
    return sgl::Rect<float>{
        .pos = {kPanelX + 24.f + static_cast<float>(index) * (width + kTabGap), kPanelY + 24.f},
        .size = {width, kTabHeight},
    };
}

[[nodiscard]] sgl::Rect<float> backRect()
{
    return sgl::Rect<float>{
        .pos = {kPanelX + (kPanelWidth - 200.f) * 0.5f, kBackY},
        .size = {200.f, 44.f},
    };
}

} // namespace

HelpScene::HelpScene(std::size_t phaseIndex, sgl::MenuActions actions, sgl::FontId font)
    : actions_{actions}
    , font_{font}
    , tabs_{uniqueTabs()}
{
    if(!tabs_.empty())
    {
        tabIndex_ = std::min(phaseIndex, tabs_.size() - 1);
    }
}

std::string_view HelpScene::tab() const
{
    if(tabs_.empty())
    {
        return {};
    }
    return tabs_[tabIndex_];
}

void HelpScene::update(sgl::SceneContext& ctx, sgl::Seconds dt)
{
    const sgl::InputState& input = ctx.input();

    if(input.action(actions_.cancel).pressed)
    {
        ctx.request(sgl::PopScene{});
        return;
    }

    if(!tabs_.empty())
    {
        if(input.action(actions_.left).pressed)
        {
            tabIndex_ = (tabIndex_ + tabs_.size() - 1) % tabs_.size();
        }
        if(input.action(actions_.right).pressed)
        {
            tabIndex_ = (tabIndex_ + 1) % tabs_.size();
        }
    }

    std::optional<std::size_t> hovered;
    if(const auto pointer = input.pointer())
    {
        for(std::size_t i = 0; i < tabs_.size(); ++i)
        {
            if(tabRect(i, tabs_.size()).contains(*pointer))
            {
                hovered = i;
                tabIndex_ = i;
                break;
            }
        }
        if(backRect().contains(*pointer))
        {
            hovered = tabs_.size();
        }
    }
    hover_.update(hovered, tabs_.size() + 1, dt);

    if(input.pointerReleased())
    {
        if(const auto pointer = input.pointer())
        {
            if(backRect().contains(*pointer))
            {
                ctx.request(sgl::PopScene{});
                return;
            }
            for(std::size_t i = 0; i < tabs_.size(); ++i)
            {
                if(tabRect(i, tabs_.size()).contains(*pointer))
                {
                    tabIndex_ = i;
                    return;
                }
            }
        }
    }

    if(input.action(actions_.confirm).pressed)
    {
        ctx.request(sgl::PopScene{});
    }
}

void HelpScene::render(sgl::RenderQueue& queue) const
{
    queue.push(sgl::Layer::Overlay,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {0.f, 0.f}, .size = {kOverlayWidth, kOverlayHeight}},
                   .fill = sgl::Color{0, 0, 0, 150},
               });

    const sgl::Rect<float> panel{.pos = {kPanelX, kPanelY}, .size = {kPanelWidth, kPanelHeight}};
    queue.push(sgl::Layer::Overlay, 1.f, sgl::roundedRect(panel, 12.f, sgl::Color{24, 28, 48, 230}));

    float depth = 2.f;
    for(std::size_t i = 0; i < tabs_.size(); ++i)
    {
        const sgl::Rect<float> rect = tabRect(i, tabs_.size());
        const sgl::Color fill =
            (i == tabIndex_) ? sgl::Color{72, 96, 160, 255} : sgl::Color{48, 56, 88, 230};
        queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(rect, 6.f, fill));
        queue.push(sgl::Layer::Overlay,
                   depth++,
                   sgl::TextCmd{
                       .font = font_,
                       .text = std::string{tabs_[i]},
                       .size = 16,
                       .position = {rect.pos.x + rect.size.x * 0.5f, rect.pos.y + rect.size.y * 0.5f},
                       .color = sgl::Color{255, 255, 255},
                       .anchor = sgl::Anchor::Center,
                   });
    }

    if(!tabs_.empty())
    {
        const std::string_view selected = tabs_[tabIndex_];
        float y = kPanelY + 80.f;
        for(const ControlRow& row: controlMap())
        {
            if(row.phase != selected)
            {
                continue;
            }
            const std::string line = std::string{row.input} + " — " + std::string{row.effect};
            queue.push(sgl::Layer::Overlay,
                       depth++,
                       sgl::TextCmd{
                           .font = font_,
                           .text = line,
                           .size = 18,
                           .position = {kPanelX + 36.f, y},
                           .color = sgl::Color{235, 238, 245},
                       });
            y += 28.f;
        }
    }

    const sgl::Rect<float> back = backRect();
    queue.push(sgl::Layer::Overlay, depth++, sgl::roundedRect(back, 8.f, sgl::Color{72, 96, 160, 255}));
    queue.push(sgl::Layer::Overlay,
               depth++,
               sgl::TextCmd{
                   .font = font_,
                   .text = "Back",
                   .size = 20,
                   .position = {back.pos.x + back.size.x * 0.5f, back.pos.y + back.size.y * 0.5f},
                   .color = sgl::Color{255, 255, 255},
                   .anchor = sgl::Anchor::Center,
               });
}

sgl::SceneTraits HelpScene::traits() const
{
    return sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
}

} // namespace sgl::syndicate
