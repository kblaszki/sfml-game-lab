#pragma once

#include <asteroids/app/Scenes.hpp>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>

namespace sgl::asteroids
{

class MainMenuScene : public sgl::SceneI
{
public:
    explicit MainMenuScene(const AppServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

private:
    AppServices services_;
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
};

} // namespace sgl::asteroids
