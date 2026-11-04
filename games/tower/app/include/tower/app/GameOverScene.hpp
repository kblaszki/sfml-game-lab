#pragma once

#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <tower/app/Scenes.hpp>

namespace sgl::tower
{

class GameOverScene : public sgl::SceneI
{
public:
    GameOverScene(const AppServices& services, int score);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] int score() const noexcept;

private:
    AppServices services_;
    int score_{};
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
};

} // namespace sgl::tower
