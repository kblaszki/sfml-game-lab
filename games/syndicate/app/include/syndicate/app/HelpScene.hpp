#pragma once

#include <cstddef>
#include <sgl/render/AssetIds.hpp>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/HoverAnim.hpp>
#include <sgl/ui/MenuActions.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace sgl::syndicate
{

class HelpScene final : public sgl::SceneI
{
public:
    HelpScene(std::size_t phaseIndex, sgl::MenuActions actions, sgl::FontId font);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    [[nodiscard]] sgl::SceneTraits traits() const override;

    [[nodiscard]] std::string_view tab() const;

private:
    sgl::MenuActions actions_;
    sgl::FontId font_{};
    std::vector<std::string_view> tabs_;
    std::size_t tabIndex_{};
    sgl::HoverAnim hover_{};
};

} // namespace sgl::syndicate
