#pragma once

#include <optional>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <string>
#include <syndicate/app/Scenes.hpp>
#include <syndicate/sim/GameState.hpp>

namespace sgl::syndicate
{

class GameOverScene final : public sgl::SceneI
{
public:
    GameOverScene(GameOver over, sgl::FontId font);
    GameOverScene(const SyndicateServices& services, GameOver over);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    [[nodiscard]] sgl::SceneTraits traits() const override;

private:
    void trySaveScore();

    std::optional<SyndicateServices> services_;
    GameOver over_;
    sgl::FontId font_{};
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
    std::string saveError_{};
};

} // namespace sgl::syndicate
