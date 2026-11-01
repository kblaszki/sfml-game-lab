#pragma once

#include <sgl/scene/SceneI.hpp>
#include <snake/app/Feedback.hpp>
#include <snake/app/Scenes.hpp>
#include <snake/sim/SnakeGame.hpp>

namespace sgl::snake
{

class PlayScene : public sgl::SceneI
{
public:
    explicit PlayScene(const AppServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] SnakeGame& game();
    [[nodiscard]] const SnakeGame& game() const;

private:
    AppServices services_;
    SnakeGame game_;
    Feedback feedback_;
};

} // namespace sgl::snake
