#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <string>
#include <tetris/app/Scenes.hpp>

namespace sgl::tetris
{

class GameOverScene : public sgl::SceneI
{
public:
    GameOverScene(const AppServices& services, std::uint32_t score);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] std::uint32_t score() const noexcept;
    [[nodiscard]] bool isNewRecord() const noexcept;
    [[nodiscard]] bool saveFailed() const noexcept;

private:
    AppServices services_;
    std::uint32_t score_{};
    std::optional<std::size_t> rank_{};
    std::string saveError_{};
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
};

} // namespace sgl::tetris
