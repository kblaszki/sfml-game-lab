#pragma once

#include <functional>
#include <optional>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/save/Settings.hpp>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/HoverAnim.hpp>
#include <sgl/ui/MenuActions.hpp>
#include <string>

namespace sgl
{

class SettingsScene final : public SceneI
{
public:
    SettingsScene(Settings initial, MenuActions actions, FontId font, std::function<void(const Settings&)> onChange);

    void update(SceneContext& ctx, Seconds dt) override;
    void render(RenderQueue& queue) const override;
    [[nodiscard]] SceneTraits traits() const override;

private:
    enum class Row : std::size_t
    {
        Volume = 0,
        Window = 1,
        Resolution = 2,
        Back = 3,
        Count = 4,
    };

    void applyChange();
    void nudge(int direction);
    [[nodiscard]] std::string rowLabel(Row row) const;
    [[nodiscard]] Rect<float> rowRect(std::size_t index) const noexcept;
    [[nodiscard]] Rect<float> leftButtonRect(std::size_t index) const noexcept;
    [[nodiscard]] Rect<float> rightButtonRect(std::size_t index) const noexcept;
    [[nodiscard]] std::optional<std::size_t> hitRow(Vec2f point) const noexcept;
    [[nodiscard]] std::optional<int> hitChevron(Vec2f point) const noexcept;

    Settings settings_;
    MenuActions actions_;
    FontId font_{};
    std::function<void(const Settings&)> onChange_;
    std::size_t focus_{};
    HoverAnim hover_{};
};

} // namespace sgl
