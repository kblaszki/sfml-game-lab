#pragma once

#include <asteroids/app/Feedback.hpp>
#include <asteroids/app/Scenes.hpp>
#include <asteroids/sim/AsteroidsGame.hpp>
#include <sgl/scene/SceneI.hpp>

namespace sgl::asteroids
{

class PlayScene : public sgl::SceneI
{
public:
    explicit PlayScene(const AppServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] AsteroidsGame& game();
    [[nodiscard]] const AsteroidsGame& game() const;

private:
    AppServices services_;
    AsteroidsGame game_;
    Feedback feedback_;
};

} // namespace sgl::asteroids
