#pragma once

#include <cstddef>
#include <optional>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <snake/app/Scenes.hpp>
#include <string>

namespace sgl::snake
{

class GameOverScene : public sgl::SceneI
{
public:
    GameOverScene(const AppServices& services, int score);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] bool isNewRecord() const noexcept;
    [[nodiscard]] bool saveFailed() const noexcept;

private:
    AppServices services_;
    int score_{};
    std::optional<std::size_t> rank_{};
    std::string saveError_{};
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
};

} // namespace sgl::snake
