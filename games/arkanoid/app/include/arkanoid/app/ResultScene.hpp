#pragma once

#include <arkanoid/app/Scenes.hpp>
#include <cstdint>
#include <optional>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>

namespace sgl::arkanoid
{

class ResultScene : public sgl::SceneI
{
public:
    ResultScene(const AppServices& services,
                Outcome outcome,
                std::uint32_t score,
                std::optional<std::size_t> rank,
                bool saveFailed);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    sgl::SceneTraits traits() const override;

    [[nodiscard]] Outcome outcome() const noexcept;
    [[nodiscard]] std::uint32_t score() const noexcept;
    [[nodiscard]] std::optional<std::size_t> rank() const noexcept;
    [[nodiscard]] bool saveFailed() const noexcept;

private:
    AppServices services_;
    Outcome outcome_{Outcome::Lost};
    std::uint32_t score_{};
    std::optional<std::size_t> rank_{};
    bool saveFailed_{false};
    sgl::MenuModel model_;
    sgl::MenuStyle style_{};
};

} // namespace sgl::arkanoid
