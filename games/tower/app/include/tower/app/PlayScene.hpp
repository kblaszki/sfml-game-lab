#pragma once

#include <sgl/scene/SceneI.hpp>
#include <tower/app/Scenes.hpp>
#include <tower/sim/TowerGame.hpp>

namespace sgl::tower
{

class PlayScene : public sgl::SceneI
{
public:
    explicit PlayScene(const AppServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] TowerGame& game();
    [[nodiscard]] const TowerGame& game() const;

private:
    AppServices services_;
    TowerGame game_;
};

} // namespace sgl::tower
