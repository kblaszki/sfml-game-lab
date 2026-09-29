#pragma once

#include <cstddef>
#include <optional>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneI.hpp>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/HudModel.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PawnAnimator.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/Bot.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Match.hpp>
#include <vector>

namespace sgl::syndicate
{

struct Seat
{
    bool bot{false};
    BotPersona persona{};
};

[[nodiscard]] std::optional<Command> commandFromInput(const Match& match,
                                                      const sgl::InputState& input,
                                                      const SyndicateActions& actions,
                                                      const LayoutView& layout);

class PlayScene final : public sgl::SceneI
{
public:
    PlayScene(MatchConfig config, std::vector<Seat> seats, sgl::FontId font, SyndicateActions actions);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    [[nodiscard]] sgl::SceneTraits traits() const override;

    [[nodiscard]] bool gameOver() const;
    [[nodiscard]] const Match* match() const noexcept;

private:
    void applyCommand(PlayerId actor, const Command& cmd);
    void tryStartPathAnimation(const ChooseDestination& dest);
    void resetPawns();

    std::vector<Seat> seats_;
    sgl::FontId font_{};
    SyndicateActions actions_{};
    Theme theme_{defaultTheme()};
    LayoutView layout_{sgl::Vec2f{640.f, 360.f}, 280.f};
    PawnAnimator animator_{layout_};
    HudModel hud_{};
    std::optional<Match> match_;
    bool setupFailed_{false};
    bool quitRequested_{false};
    bool endPushed_{false};
    float botThink_{0.f};
};

} // namespace sgl::syndicate
