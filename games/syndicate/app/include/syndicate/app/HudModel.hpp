#pragma once

#include <cstddef>
#include <sgl/core/Time.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <string>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Match.hpp>
#include <vector>

namespace sgl::syndicate
{

struct HudNote
{
    std::string text;
    float ttl{};
};

class HudModel
{
public:
    void pushNote(std::string text, float ttl);
    void update(sgl::Seconds dt);
    void render(sgl::RenderQueue& queue, const Match& match, sgl::FontId font, const Theme& theme) const;

    [[nodiscard]] std::string phasePrompt(const Phase& phase) const;
    [[nodiscard]] std::size_t noteCount() const noexcept;

private:
    std::vector<HudNote> notes_;
};

} // namespace sgl::syndicate
