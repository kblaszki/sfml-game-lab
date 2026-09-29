#include <syndicate/app/PlayScene.hpp>

#include <cstddef>
#include <memory>
#include <sgl/core/Color.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <sgl/scene/SceneRequest.hpp>
#include <string>
#include <syndicate/app/BoardRender.hpp>
#include <syndicate/app/GameOverScene.hpp>
#include <syndicate/app/Scenes.hpp>
#include <utility>
#include <variant>

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
}

PlayScene::PlayScene(const SyndicateServices& services, MatchConfig config, std::vector<Seat> seats)
    : seats_{std::move(seats)}
    , font_{services.font}
    , actions_{services.actions}
    , services_{services}
    , feedback_{Feedback{services.audio, services.sounds}}
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
    animator_.start(match_->actingPlayer(), points);
}

void PlayScene::drainFeedback()
{
    if(!match_.has_value() || !feedback_.has_value())
    {
        return;
    }
    const std::vector<GameEvent> events = match_->drainEvents();
    feedback_->onEvents(events);
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
    botThink_ = 0.f;
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

    animator_.update(dt);
    hud_.update(dt);

    if(!gameOver())
    {
        const sgl::InputState& input = ctx.input();
        if(input.action(actions_.pause).pressed)
        {
            ctx.request(sgl::RequestPause{});
        }

        if(!animator_.busy())
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
                else if(const auto cmd = commandFromInput(*match_, input, actions_, layout_))
                {
                    applyCommand(actor, *cmd);
                }
            }
        }
    }

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
            ctx.request(sgl::PushScene{[over, font]() {
                return std::make_unique<GameOverScene>(over, font);
            }});
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
    renderBoard(queue, s.board, s.ownership, layout_, theme_, highlighted);

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
}

sgl::SceneTraits PlayScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = true};
}

} // namespace sgl::syndicate
