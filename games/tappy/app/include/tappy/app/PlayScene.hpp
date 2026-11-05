#pragma once

#include <sgl/scene/SceneI.hpp>
#include <tappy/app/Feedback.hpp>
#include <tappy/app/Scenes.hpp>
#include <tappy/sim/TappyGame.hpp>

namespace sgl::tappy
{

class PlayScene : public sgl::SceneI
{
public:
    explicit PlayScene(const AppServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] TappyGame& game();
    [[nodiscard]] const TappyGame& game() const;

private:
    AppServices services_;
    TappyGame game_;
    Feedback feedback_;
};

} // namespace sgl::tappy
