#pragma once

#include <sgl/render/AssetIds.hpp>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuActions.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>

namespace sgl
{

class PauseOverlay final : public SceneI
{
public:
    PauseOverlay(MenuActions actions, FontId font);

    void update(SceneContext& ctx, Seconds dt) override;
    void render(RenderQueue& queue) const override;
    [[nodiscard]] SceneTraits traits() const override;

private:
    MenuActions actions_;
    MenuModel model_;
    MenuStyle style_;
};

} // namespace sgl
