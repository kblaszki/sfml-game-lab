#pragma once

#include <cstdint>
#include <expected>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/Events.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Ids.hpp>
#include <vector>

namespace sgl::syndicate
{

enum class SetupError
{
    BadSeats,
};

enum class CommandError
{
    NotYourTurn,
    WrongPhase,
    InvalidArgument,
    InsufficientFunds,
};

class Match
{
public:
    [[nodiscard]] static std::expected<Match, SetupError> create(const MatchConfig& cfg);

    [[nodiscard]] std::expected<void, CommandError> apply(PlayerId actor, const Command& cmd);
    [[nodiscard]] std::vector<Command> legalCommands(PlayerId actor) const;
    [[nodiscard]] PlayerId actingPlayer() const noexcept;
    [[nodiscard]] const GameState& state() const noexcept;
    [[nodiscard]] std::vector<GameEvent> drainEvents();

private:
    explicit Match(GameState state);

    GameState state_;
};

} // namespace sgl::syndicate
