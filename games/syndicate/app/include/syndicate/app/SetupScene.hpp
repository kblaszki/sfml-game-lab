#pragma once

#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <syndicate/app/PlayScene.hpp>
#include <syndicate/app/Scenes.hpp>
#include <vector>

namespace sgl::syndicate
{

class SetupScene final : public sgl::SceneI
{
public:
    explicit SetupScene(const SyndicateServices& services);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    [[nodiscard]] sgl::SceneTraits traits() const override;

private:
    void rebuildModel();
    [[nodiscard]] MatchConfig makeConfig() const;
    [[nodiscard]] std::vector<Seat> makeSeats() const;

    SyndicateServices services_;
    int seatCount_{2};
    std::vector<bool> bots_{false, false, false, false};
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
};

} // namespace sgl::syndicate
