#pragma once

#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <syndicate/app/Scenes.hpp>

namespace sgl::syndicate
{

class PauseMenuScene final : public sgl::SceneI
{
public:
    explicit PauseMenuScene(const SyndicateServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    [[nodiscard]] sgl::SceneTraits traits() const override;

private:
    SyndicateServices services_;
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
};

} // namespace sgl::syndicate
