#include <syndicate/app/ControlMap.hpp>

#include <array>
#include <type_traits>
#include <variant>

namespace sgl::syndicate
{
namespace
{

constexpr std::array<ControlRow, 16> kControlRows{{
    {.phase = "Roll", .input = "Space or [1]", .effect = "roll the dice"},
    {.phase = "Path", .input = "click a glowing tile", .effect = "choose destination"},
    {.phase = "Path", .input = "momentum buttons", .effect = "spend momentum on a path"},
    {.phase = "Purchase", .input = "[1]-[9] buy", .effect = "buy shares on the tile"},
    {.phase = "Purchase", .input = "Enter decline", .effect = "skip the purchase"},
    {.phase = "Auction", .input = "C min bid", .effect = "place the minimum bid"},
    {.phase = "Auction", .input = "number keys", .effect = "raise or pick a bid slot"},
    {.phase = "Auction", .input = "P pass", .effect = "pass the auction"},
    {.phase = "Trade", .input = "accept/reject buttons", .effect = "answer a trade offer"},
    {.phase = "Manage", .input = "click a tile to filter", .effect = "focus legal actions"},
    {.phase = "Manage", .input = "number keys", .effect = "pick an action-bar command"},
    {.phase = "Manage", .input = "Enter end turn", .effect = "finish the manage phase"},
    {.phase = "Global", .input = "Esc pause", .effect = "open the pause menu"},
    {.phase = "Global", .input = "H help", .effect = "open contextual help"},
    {.phase = "Global", .input = "hover for tooltips", .effect = "read tile and button tips"},
    {.phase = "Global",
     .input = "Up Down Left Right Backspace [1] [2] [3] [4] [5] [6] [7] [8] [9]",
     .effect = "navigate menus, cancel, and action-bar slots"},
}};

} // namespace

std::span<const ControlRow> controlMap()
{
    return kControlRows;
}

std::string hintLine(const Phase& phase)
{
    return std::visit(
        [](const auto& p) -> std::string {
            using T = std::decay_t<decltype(p)>;
            if constexpr(std::is_same_v<T, AwaitRoll>)
            {
                return "Space or [1] to roll";
            }
            else if constexpr(std::is_same_v<T, AwaitPath>)
            {
                return "Click a glowing tile; use momentum buttons if offered";
            }
            else if constexpr(std::is_same_v<T, AwaitPurchase>)
            {
                return "[1]-[9] buy shares, Enter to decline";
            }
            else if constexpr(std::is_same_v<T, InAuction>)
            {
                return "C min bid, number keys to bid, P to pass";
            }
            else if constexpr(std::is_same_v<T, AwaitTradeResponse>)
            {
                return "Use accept/reject buttons on the action bar";
            }
            else if constexpr(std::is_same_v<T, Manage>)
            {
                return "Click a tile to filter, number keys, Enter ends turn";
            }
            else if constexpr(std::is_same_v<T, GameOver>)
            {
                return "Match over — Esc pause, H help, hover for tooltips";
            }
            else
            {
                return "H help";
            }
        },
        phase);
}

} // namespace sgl::syndicate
