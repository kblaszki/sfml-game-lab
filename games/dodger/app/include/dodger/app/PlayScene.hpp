#pragma once

#include <dodger/app/Feedback.hpp>
#include <dodger/app/Scenes.hpp>
#include <dodger/sim/DodgerGame.hpp>
#include <sgl/scene/SceneI.hpp>

namespace sgl::dodger
{

class PlayScene : public sgl::SceneI
{
public:
    explicit PlayScene(const AppServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] DodgerGame& game();
    [[nodiscard]] const DodgerGame& game() const;

private:
    AppServices services_;
    DodgerGame game_;
    Feedback feedback_;
};

} // namespace sgl::dodger
