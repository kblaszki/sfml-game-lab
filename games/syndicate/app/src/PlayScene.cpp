#include <cstddef>
#include <memory>
#include <optional>
#include <sgl/core/Color.hpp>
#include <sgl/core/Overloaded.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/scene/SceneRequest.hpp>
#include <sgl/ui/MenuStyle.hpp>
#include <string>
#include <syndicate/app/BoardRender.hpp>
#include <syndicate/app/ControlMap.hpp>
#include <syndicate/app/Describe.hpp>
#include <syndicate/app/GameOverScene.hpp>
#include <syndicate/app/HelpScene.hpp>
#include <syndicate/app/PlayScene.hpp>
#include <syndicate/app/Scenes.hpp>
#include <syndicate/sim/Events.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] std::size_t seatIndex(PlayerId id) noexcept
{
    return static_cast<std::size_t>(std::to_underlying(id));
}

[[nodiscard]] const MoveOption* findOption(const AwaitPath& path, NodeId destination, bool passesPayday)
{
    for(const MoveOption& opt: path.options)
    {
        if(opt.destination == destination && opt.passesPayday == passesPayday)
        {
            return &opt;
        }
    }
    return nullptr;
}

[[nodiscard]] const MoveOption* findOptionByDestination(const AwaitPath& path, NodeId destination)
{
    for(const MoveOption& opt: path.options)
    {
        if(opt.destination == destination)
        {
            return &opt;
        }
    }
    return nullptr;
}

[[nodiscard]] std::optional<Bid> smallestBid(const std::vector<Command>& legal)
{
    std::optional<Bid> best;
    for(const Command& cmd: legal)
    {
        if(const Bid* bid = std::get_if<Bid>(&cmd))
        {
            if(!best.has_value() || bid->amount < best->amount)
            {
                best = *bid;
            }
        }
    }
    return best;
}

[[nodiscard]] bool hasDecline(const std::vector<Command>& legal)
{
    for(const Command& cmd: legal)
    {
        if(std::holds_alternative<DeclinePurchase>(cmd))
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool hasBuyOne(const std::vector<Command>& legal)
{
    for(const Command& cmd: legal)
    {
        if(const auto* buy = std::get_if<BuyShares>(&cmd))
        {
            if(buy->count == 1)
            {
                return true;
            }
        }
    }
    return false;
}

[[nodiscard]] bool pointerOnActionBar(const ActionBar& bar, sgl::Vec2f pointer)
{
    for(const sgl::Hotspot& spot: bar.hotspots())
    {
        if(spot.area.contains(pointer))
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool offerTradeSlot(const Match& match)
{
    const auto* manage = std::get_if<Manage>(&match.state().phase);
    return manage != nullptr && manage->tradesProposed < 3;
}

[[nodiscard]] sgl::MenuStyle makeActionBarStyle(sgl::FontId font)
{
    return sgl::MenuStyle{
        .font = font,
        .textSize = 14,
        .idle = sgl::Color{48, 54, 72, 230},
        .focused = sgl::Color{70, 90, 130, 240},
        .text = sgl::Color{235, 238, 245},
        .panel = sgl::Color{20, 24, 36, 200},
        .cornerRadius = 6.f,
        .bodyFont = {},
    };
}

[[nodiscard]] std::size_t helpTabIndex(const Phase& phase) noexcept
{
    if(std::holds_alternative<AwaitRoll>(phase))
    {
        return 0;
    }
    if(std::holds_alternative<AwaitPath>(phase))
    {
        return 1;
    }
    if(std::holds_alternative<AwaitPurchase>(phase))
    {
        return 2;
    }
    if(std::holds_alternative<InAuction>(phase))
    {
        return 3;
    }
    if(std::holds_alternative<AwaitTradeResponse>(phase))
    {
        return 4;
    }
    if(std::holds_alternative<Manage>(phase))
    {
        return 5;
    }
    return 6; // GameOver / Global
}

} // namespace

std::optional<Command> commandFromInput(const Match& match,
                                        const sgl::InputState& input,
                                        const SyndicateActions& actions,
                                        const LayoutView& layout)
{
    const PlayerId actor = match.actingPlayer();
    const Phase& phase = match.state().phase;
    const std::vector<Command> legal = match.legalCommands(actor);

    if(std::holds_alternative<AwaitRoll>(phase))
    {
        if(input.action(actions.roll).pressed)
        {
            return RollDice{};
        }
        return std::nullopt;
    }

    if(const auto* path = std::get_if<AwaitPath>(&phase))
    {
        if(input.pointerReleased())
        {
            const auto pointer = input.pointer();
            if(pointer.has_value())
            {
                const float maxDist = layout.nodeRadius() * 1.5f;
                const float maxDistSq = maxDist * maxDist;
                for(const MoveOption& opt: path->options)
                {
                    const sgl::Vec2f center = layout.position(match.state().board.node(opt.destination).at);
                    if(sgl::lengthSq(*pointer - center) <= maxDistSq)
                    {
                        return ChooseDestination{.destination = opt.destination, .passesPayday = opt.passesPayday};
                    }
                }
            }
        }
        return std::nullopt;
    }

    if(std::holds_alternative<Manage>(phase))
    {
        if(input.action(actions.endTurn).pressed)
        {
            return EndTurn{};
        }
        return std::nullopt;
    }

    if(std::holds_alternative<InAuction>(phase))
    {
        if(input.action(actions.bid).pressed)
        {
            if(const auto bid = smallestBid(legal))
            {
                return *bid;
            }
        }
        if(input.action(actions.passBid).pressed)
        {
            return PassBid{};
        }
        return std::nullopt;
    }

    if(std::holds_alternative<AwaitPurchase>(phase))
    {
        if(input.action(actions.endTurn).pressed && hasDecline(legal))
        {
            return DeclinePurchase{};
        }
        if(input.action(actions.roll).pressed && hasBuyOne(legal))
        {
            return BuyShares{.count = 1};
        }
        return std::nullopt;
    }

    return std::nullopt;
}

PlayScene::PlayScene(MatchConfig config, std::vector<Seat> seats, sgl::FontId font, SyndicateActions actions)
    : seats_{std::move(seats)}
    , font_{font}
    , actions_{actions}
    , presentation_{animator_, config.seed}
{
    if(std::cmp_not_equal(seats_.size(), config.seats))
    {
        setupFailed_ = true;
        return;
    }
    auto created = Match::create(config);
    if(!created.has_value())
    {
        setupFailed_ = true;
        return;
    }
    match_ = std::move(*created);
    resetPawns();
    hud_.recordMarket(match_->state().market);
}

PlayScene::PlayScene(const SyndicateServices& services, MatchConfig config, std::vector<Seat> seats)
    : seats_{std::move(seats)}
    , font_{services.font}
    , actions_{services.actions}
    , services_{services}
    , feedback_{Feedback{services.audio, services.sounds}}
    , presentation_{animator_, services.seed}
{
    if(std::cmp_not_equal(seats_.size(), config.seats))
    {
        setupFailed_ = true;
        return;
    }
    auto created = Match::create(config);
    if(!created.has_value())
    {
        setupFailed_ = true;
        return;
    }
    match_ = std::move(*created);
    resetPawns();
    hud_.recordMarket(match_->state().market);
}

void PlayScene::resetPawns()
{
    if(!match_.has_value())
    {
        return;
    }
    const GameState& s = match_->state();
    for(int i = 0; i < s.seats; ++i)
    {
        const PlayerId id = PlayerId{static_cast<std::uint8_t>(i)};
        const NodeId pos = s.position[static_cast<std::size_t>(i)];
        animator_.reset(id, s.board.node(pos).at);
    }
}

void PlayScene::tryStartPathAnimation(const ChooseDestination& dest)
{
    if(!match_.has_value())
    {
        return;
    }
    const auto* path = std::get_if<AwaitPath>(&match_->state().phase);
    if(!path)
    {
        return;
    }
    const MoveOption* opt = findOption(*path, dest.destination, dest.passesPayday);
    if(!opt)
    {
        opt = findOptionByDestination(*path, dest.destination);
    }
    if(!opt)
    {
        return;
    }
    std::vector<sgl::Vec2f> points;
    points.reserve(opt->path.size());
    for(const NodeId node: opt->path)
    {
        points.push_back(layout_.position(match_->state().board.node(node).at));
    }
    presentation_.onPath(match_->actingPlayer(), points);
}

void PlayScene::drainFeedback()
{
    if(!match_.has_value())
    {
        return;
    }
    const std::vector<GameEvent> events = match_->drainEvents();
    for(const GameEvent& event: events)
    {
        if(const auto* rolled = std::get_if<Rolled>(&event))
        {
            presentation_.onRolled(rolled->roll);
        }
    }
    if(feedback_.has_value())
    {
        feedback_->onEvents(events);
    }

    const int round = match_->state().round;
    if(round != lastLoggedRound_)
    {
        lastLoggedRound_ = round;
        log_.push(round, std::nullopt, "— Round " + std::to_string(round) + " —");
    }
    for(const GameEvent& event: events)
    {
        const std::optional<PlayerId> player =
            std::visit(sgl::Overloaded{
                           [](const Rolled& e) -> std::optional<PlayerId> { return e.player; },
                           [](const Moved& e) -> std::optional<PlayerId> { return e.player; },
                           [](const PaydayPaid& e) -> std::optional<PlayerId> { return e.player; },
                           [](const FeePaid& e) -> std::optional<PlayerId> { return e.payer; },
                           [](const Liquidated& e) -> std::optional<PlayerId> { return e.player; },
                           [](const SharesBought& e) -> std::optional<PlayerId> { return e.player; },
                           [](const AuctionDone& e) -> std::optional<PlayerId> { return e.winner; },
                           [](const CardPlayed& e) -> std::optional<PlayerId> { return e.player; },
                           [](const TradeResolved& e) -> std::optional<PlayerId> { return e.from; },
                           [](const MarketShifted&) -> std::optional<PlayerId> { return std::nullopt; },
                           [](const GameEnded&) -> std::optional<PlayerId> { return std::nullopt; },
                       },
                       event);
        const std::string line = describeEvent(match_->state(), event);
        log_.push(round, player, line);
        std::visit(sgl::Overloaded{
                       [this](const PaydayPaid&) { hud_.pushNote("Payday settled", 2.5f); },
                       [this](const TradeResolved&) { hud_.pushNote("Trade resolved", 2.5f); },
                       [this](const AuctionDone&) { hud_.pushNote("Auction closed", 2.5f); },
                       [](const auto&) {},
                   },
                   event);
    }

    if(match_->state().revealed.has_value())
    {
        if(lastRevealNote_ != match_->state().revealed)
        {
            lastRevealNote_ = match_->state().revealed;
            hud_.pushNote("Insider peek ready", 3.f);
        }
    }
    else
    {
        lastRevealNote_.reset();
    }
}

void PlayScene::applyCommand(PlayerId actor, const Command& cmd)
{
    Match* match = match_ ? &*match_ : nullptr;
    if(match == nullptr)
    {
        return;
    }
    if(const auto* choose = std::get_if<ChooseDestination>(&cmd))
    {
        tryStartPathAnimation(*choose);
    }
    if(const auto applied = match->apply(actor, cmd); !applied)
    {
        return;
    }
    drainFeedback();
    hud_.recordMarket(match->state().market);
    botThink_ = 0.f;
}

void PlayScene::updateHover(const sgl::InputState& input, sgl::Seconds dt)
{
    std::optional<std::size_t> hovered;
    if(const auto pointer = input.pointer())
    {
        const auto spots = actionBar_.hotspots();
        for(std::size_t i = 0; i < spots.size(); ++i)
        {
            if(spots[i].area.contains(*pointer))
            {
                hovered = i;
                break;
            }
        }
    }
    hover_.update(hovered, actionBar_.size(), dt);
}

void PlayScene::trySelectTile(const sgl::InputState& input)
{
    if(!match_.has_value() || !input.pointerReleased())
    {
        return;
    }
    const Phase& phase = match_->state().phase;
    if(!std::holds_alternative<Manage>(phase) && !std::holds_alternative<AwaitPurchase>(phase))
    {
        return;
    }
    const auto pointer = input.pointer();
    if(!pointer.has_value() || pointerOnActionBar(actionBar_, *pointer))
    {
        return;
    }

    const float maxDist = layout_.nodeRadius() * 1.5f;
    const float maxDistSq = maxDist * maxDist;
    std::optional<NodeId> hit;
    float bestDistSq = maxDistSq;
    for(std::size_t i = 0; i < match_->state().board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const sgl::Vec2f center = layout_.position(match_->state().board.node(id).at);
        const float distSq = sgl::lengthSq(*pointer - center);
        if(distSq <= bestDistSq)
        {
            bestDistSq = distSq;
            hit = id;
        }
    }
    if(hit.has_value())
    {
        selectedTile_ = hit;
        actionBar_.rebuild(*match_, match_->actingPlayer(), selectedTile_, offerTradeSlot(*match_));
    }
}

sgl::FontId PlayScene::tooltipFont() const noexcept
{
    if(services_.has_value() && services_->bodyFont.id != 0)
    {
        return services_->bodyFont;
    }
    return font_;
}

void PlayScene::updateTooltips(const sgl::InputState& input, sgl::Seconds dt)
{
    if(!match_.has_value())
    {
        return;
    }

    const GameState& state = match_->state();
    const PlayerId actor = match_->actingPlayer();
    const std::size_t actorIdx = seatIndex(actor);
    if(actorIdx < state.hands.size())
    {
        std::vector<CardKind> playable;
        for(const Command& cmd: match_->legalCommands(actor))
        {
            if(const auto* play = std::get_if<PlayCard>(&cmd))
            {
                playable.push_back(play->kind);
            }
        }
        handView_.setPlayableKinds(playable);
        handView_.sync(state.hands[actorIdx], dt);
    }

    std::vector<sgl::Hotspot> hotspots;
    const float side = layout_.nodeRadius() * 2.f;
    for(std::size_t i = 0; i < state.board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const sgl::Vec2f center = layout_.position(state.board.node(id).at);
        const std::string desc = describeTile(state, id);
        hotspots.push_back(sgl::Hotspot{
            .area = sgl::Rect<float>{.pos = {center.x - side * 0.5f, center.y - side * 0.5f}, .size = {side, side}},
            .title = desc,
            .body = desc,
        });
    }

    for(const sgl::Hotspot& spot: actionBar_.hotspots())
    {
        hotspots.push_back(spot);
    }
    for(const sgl::Hotspot& spot: handView_.hotspots())
    {
        hotspots.push_back(spot);
    }
    for(const sgl::Hotspot& spot: hud_.chartHotspots())
    {
        hotspots.push_back(spot);
    }
    for(int i = 0; i < state.seats; ++i)
    {
        const std::string tip = hud_.playerTooltip(*match_, i);
        hotspots.push_back(sgl::Hotspot{
            .area = playerPanelRect(i),
            .title = "P" + std::to_string(i + 1),
            .body = tip,
        });
    }

    const sgl::Vec2f pointer = input.pointer().value_or(sgl::Vec2f{0.f, 0.f});
    const sgl::TooltipStyle style{.font = tooltipFont()};
    tooltip_.update(pointer, hotspots, dt, style);
}

const Match* PlayScene::match() const noexcept
{
    return match_.has_value() ? &*match_ : nullptr;
}

bool PlayScene::gameOver() const
{
    return match_.has_value() && std::holds_alternative<GameOver>(match_->state().phase);
}

void PlayScene::update(sgl::SceneContext& ctx, sgl::Seconds dt)
{
    if(setupFailed_)
    {
        if(!quitRequested_)
        {
            quitRequested_ = true;
            ctx.request(sgl::QuitApp{});
        }
        return;
    }
    if(!match_.has_value())
    {
        return;
    }

    presentation_.update(dt);
    hud_.update(dt);
    if(!onboardingHintsDone_)
    {
        onboardingHintsDone_ = true;
        hud_.pushNote(hintLine(match_->state().phase), 4.f);
        hud_.pushNote("Press H for phase help", 4.f);
    }

    const sgl::InputState& input = ctx.input();
    const sgl::Vec2f pointer = input.pointer().value_or(sgl::Vec2f{0.f, 0.f});

    if(const auto* path = std::get_if<AwaitPath>(&match_->state().phase))
    {
        highlighter_.build(path->options, match_->state().board, layout_);
    }
    else
    {
        highlighter_.build({}, match_->state().board, layout_);
    }
    highlighter_.update(dt, pointer);

    if(!gameOver())
    {
        const bool closeCompose = tradeCompose_.active() && input.action(actions_.pause).pressed;
        if(closeCompose)
        {
            tradeCompose_.close();
        }
        else if(input.action(actions_.pause).pressed)
        {
            ctx.request(sgl::RequestPause{});
        }

        if(!closeCompose && presentation_.busy())
        {
            if(input.pointerPressed() || input.action(actions_.roll).pressed)
            {
                presentation_.fastForward();
            }
        }
        else if(!closeCompose)
        {
            if(input.action(actions_.help).pressed)
            {
                const std::size_t tab = helpTabIndex(match_->state().phase);
                ctx.request(sgl::PushScene{[tab, actions = actions_.menu, font = font_]() {
                    return std::make_unique<HelpScene>(tab, actions, font);
                }});
            }
            else
            {
                const PlayerId actor = match_->actingPlayer();
                const std::size_t idx = seatIndex(actor);
                if(idx < seats_.size())
                {
                    if(seats_[idx].bot)
                    {
                        botThink_ += dt.count();
                        if(botThink_ >= 0.4f)
                        {
                            const Command cmd = chooseCommand(*match_, actor, seats_[idx].persona);
                            applyCommand(actor, cmd);
                        }
                    }
                    else if(tradeCompose_.active())
                    {
                        tradeCompose_.update(input, *match_, layout_, actions_);
                    }
                    else
                    {
                        actionBar_.rebuild(*match_, actor, selectedTile_, offerTradeSlot(*match_));
                        trySelectTile(input);
                        if(const auto barCmd = actionBar_.update(input, actions_))
                        {
                            applyCommand(actor, *barCmd);
                        }
                        else if(actionBar_.consumeTradeRequest())
                        {
                            tradeCompose_.open(*match_, actor);
                        }
                        else if(const auto cmd = commandFromInput(*match_, input, actions_, layout_))
                        {
                            applyCommand(actor, *cmd);
                        }
                    }
                }
            }
        }
        updateHover(input, dt);
    }

    updateTooltips(input, dt);

    if(gameOver() && !endPushed_)
    {
        const Match* match = match_ ? &*match_ : nullptr;
        if(match == nullptr)
        {
            return;
        }
        const auto* overPtr = std::get_if<GameOver>(&match->state().phase);
        if(overPtr == nullptr)
        {
            return;
        }
        endPushed_ = true;
        const GameOver& over = *overPtr;
        if(services_.has_value())
        {
            ctx.request(sgl::PushScene{makeGameOver(*services_, over)});
        }
        else
        {
            const sgl::FontId font = font_;
            ctx.request(sgl::PushScene{[over, font]() { return std::make_unique<GameOverScene>(over, font); }});
        }
    }
}

void PlayScene::render(sgl::RenderQueue& queue) const
{
    if(!match_.has_value())
    {
        return;
    }
    const GameState& s = match_->state();
    std::vector<MoveOption> highlighted;
    if(const auto* path = std::get_if<AwaitPath>(&s.phase))
    {
        highlighted = path->options;
    }
    const SpriteBank* sprites = services_.has_value() ? &services_->sprites : nullptr;
    const HighlightModel* highlights = nullptr;
    if(std::holds_alternative<AwaitPath>(s.phase))
    {
        highlights = &highlighter_.model();
    }
    renderBoard(queue, s.board, s.ownership, layout_, theme_, highlighted, font_, sprites, highlights, selectedTile_);
    if(highlights != nullptr)
    {
        highlighter_.render(queue, theme_);
    }

    for(int i = 0; i < s.seats; ++i)
    {
        const PlayerId id = PlayerId{static_cast<std::uint8_t>(i)};
        queue.push(sgl::Layer::Actors,
                   static_cast<float>(i),
                   sgl::CircleCmd{
                       .center = animator_.position(id),
                       .radius = 10.f,
                       .fill = theme_.pawn[static_cast<std::size_t>(i)],
                       .outline = sgl::Color{20, 20, 30},
                       .outlineThickness = 2.f,
                   });
    }

    hud_.render(queue, *match_, font_, theme_);
    queue.push(sgl::Layer::Hud,
               50.f,
               sgl::TextCmd{
                   .font = font_,
                   .text = hintLine(match_->state().phase),
                   .size = 15,
                   .position = sgl::Vec2f{24.f, 628.f},
                   .color = sgl::Color{200, 210, 225},
               });
    presentation_.render(queue, layout_.origin(), theme_);
    handView_.render(queue, font_, theme_);
    actionBar_.render(queue, makeActionBarStyle(font_), hover_);
    log_.render(queue, kRightLog, theme_, font_);
    auctionPanel_.render(queue, *match_, font_, theme_);
    tradeRespond_.render(queue, *match_, font_, theme_);
    tradeCompose_.render(queue, font_, theme_);
    insiderBanner_.render(queue, match_->state(), font_, theme_);
    tooltip_.render(queue, sgl::TooltipStyle{.font = tooltipFont()});
}

sgl::SceneTraits PlayScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = true};
}

} // namespace sgl::syndicate
