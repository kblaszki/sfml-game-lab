#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <sgl/audio/AudioI.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/save/Settings.hpp>
#include <sgl/scene/SceneI.hpp>
#include <sgl/ui/HoverAnim.hpp>
#include <sgl/ui/TooltipSystem.hpp>
#include <syndicate/app/ActionBar.hpp>
#include <syndicate/app/ActivityLog.hpp>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/Feedback.hpp>
#include <syndicate/app/HandView.hpp>
#include <syndicate/app/HudModel.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PathHighlighter.hpp>
#include <syndicate/app/PawnAnimator.hpp>
#include <syndicate/app/Presentation.hpp>
#include <syndicate/app/Sounds.hpp>
#include <syndicate/app/SpriteBank.hpp>
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

// References owned by main() for the whole App::run(); scenes must not outlive that.
struct SyndicateServices
{
    const SyndicateActions& actions;
    sgl::FontId font;
    std::uint64_t seed{};
    sgl::AudioI& audio;
    SoundIds sounds{};
    std::filesystem::path scoresPath{};
    sgl::HighScoreTable& highScores;
    SpriteBank sprites{};
    sgl::FontId bodyFont{};
    sgl::Settings settings{};
    std::function<void(const sgl::Settings&)> onSettingsChanged{};
};

[[nodiscard]] std::optional<Command> commandFromInput(const Match& match,
                                                      const sgl::InputState& input,
                                                      const SyndicateActions& actions,
                                                      const LayoutView& layout);

class PlayScene final : public sgl::SceneI
{
public:
    PlayScene(MatchConfig config, std::vector<Seat> seats, sgl::FontId font, SyndicateActions actions);
    PlayScene(const SyndicateServices& services, MatchConfig config, std::vector<Seat> seats);

    void update(sgl::SceneContext& ctx, sgl::Seconds dt) override;
    void render(sgl::RenderQueue& queue) const override;
    [[nodiscard]] sgl::SceneTraits traits() const override;

    [[nodiscard]] bool gameOver() const;
    [[nodiscard]] const Match* match() const noexcept;
    [[nodiscard]] const ActivityLog& log() const noexcept
    {
        return log_;
    }

private:
    void applyCommand(PlayerId actor, const Command& cmd);
    void tryStartPathAnimation(const ChooseDestination& dest);
    void resetPawns();
    void drainFeedback();
    void updateHover(const sgl::InputState& input, sgl::Seconds dt);
    void trySelectTile(const sgl::InputState& input);
    void updateTooltips(const sgl::InputState& input, sgl::Seconds dt);
    [[nodiscard]] sgl::FontId tooltipFont() const noexcept;

    std::vector<Seat> seats_;
    sgl::FontId font_{};
    SyndicateActions actions_{};
    std::optional<SyndicateServices> services_;
    std::optional<Feedback> feedback_;
    Theme theme_{defaultTheme()};
    LayoutView layout_{kBoardOrigin, kBoardRadius};
    PawnAnimator animator_{layout_};
    Presentation presentation_;
    HudModel hud_{};
    ActivityLog log_{};
    int lastLoggedRound_{0};
    PathHighlighter highlighter_{};
    ActionBar actionBar_{};
    HandView handView_{};
    sgl::HoverAnim hover_{};
    sgl::TooltipSystem tooltip_{};
    std::optional<NodeId> selectedTile_{};
    std::optional<Match> match_;
    bool setupFailed_{false};
    bool quitRequested_{false};
    bool endPushed_{false};
    float botThink_{0.f};
};

} // namespace sgl::syndicate
