#include <gtest/gtest.h>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/Key.hpp>
#include <string>
#include <string_view>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/ControlMap.hpp>
#include <syndicate/sim/GameState.hpp>
#include <variant>
#include <vector>

namespace
{

[[nodiscard]] std::string_view keyName(sgl::Key key)
{
    using sgl::Key;
    switch(key)
    {
    case Key::Space:
        return "Space";
    case Key::Enter:
        return "Enter";
    case Key::C:
        return "C";
    case Key::P:
        return "P";
    case Key::Escape:
        return "Esc";
    case Key::H:
        return "H";
    case Key::Up:
        return "Up";
    case Key::Down:
        return "Down";
    case Key::Left:
        return "Left";
    case Key::Right:
        return "Right";
    case Key::Backspace:
        return "Backspace";
    case Key::Num1:
        return "[1]";
    case Key::Num2:
        return "[2]";
    case Key::Num3:
        return "[3]";
    case Key::Num4:
        return "[4]";
    case Key::Num5:
        return "[5]";
    case Key::Num6:
        return "[6]";
    case Key::Num7:
        return "[7]";
    case Key::Num8:
        return "[8]";
    case Key::Num9:
        return "[9]";
    default:
        return {};
    }
}

[[nodiscard]] std::string controlMapText()
{
    std::string text;
    for(const auto& row: sgl::syndicate::controlMap())
    {
        text += row.phase;
        text += ' ';
        text += row.input;
        text += ' ';
        text += row.effect;
        text += ' ';
    }
    return text;
}

[[nodiscard]] bool phaseMentioned(std::string_view name)
{
    for(const auto& row: sgl::syndicate::controlMap())
    {
        if(row.phase == name)
        {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(ControlMapShould, mentionEveryPhaseAlternative)
{
    EXPECT_TRUE(phaseMentioned("Roll"));
    EXPECT_TRUE(phaseMentioned("Path"));
    EXPECT_TRUE(phaseMentioned("Purchase"));
    EXPECT_TRUE(phaseMentioned("Auction"));
    EXPECT_TRUE(phaseMentioned("Trade"));
    EXPECT_TRUE(phaseMentioned("Manage"));
    EXPECT_TRUE(phaseMentioned("Global"));

    // Default-constructed alternatives exist as distinct types in Phase.
    using sgl::syndicate::AwaitPath;
    using sgl::syndicate::AwaitPurchase;
    using sgl::syndicate::AwaitRoll;
    using sgl::syndicate::AwaitTradeResponse;
    using sgl::syndicate::GameOver;
    using sgl::syndicate::InAuction;
    using sgl::syndicate::Manage;
    using sgl::syndicate::Phase;

    const Phase phases[] = {
        AwaitRoll{},
        AwaitPath{},
        AwaitPurchase{},
        InAuction{},
        AwaitTradeResponse{},
        Manage{},
        GameOver{},
    };
    EXPECT_EQ(std::variant_size_v<Phase>, 7u);
    EXPECT_EQ(std::size(phases), 7u);
}

TEST(ControlMapShould, hintLineNonEmptyAndBounded)
{
    using sgl::syndicate::AwaitPath;
    using sgl::syndicate::AwaitPurchase;
    using sgl::syndicate::AwaitRoll;
    using sgl::syndicate::AwaitTradeResponse;
    using sgl::syndicate::GameOver;
    using sgl::syndicate::InAuction;
    using sgl::syndicate::Manage;
    using sgl::syndicate::hintLine;

    const sgl::syndicate::Phase phases[] = {
        AwaitRoll{},
        AwaitPath{},
        AwaitPurchase{},
        InAuction{},
        AwaitTradeResponse{},
        Manage{},
        GameOver{},
    };

    for(const auto& phase: phases)
    {
        const std::string line = hintLine(phase);
        EXPECT_FALSE(line.empty());
        EXPECT_LE(line.size(), 90u);
    }
}

TEST(ControlMapShould, includeEveryBoundKeyName)
{
    sgl::ActionMap map;
    const auto actions = sgl::syndicate::bindSyndicate(map);
    const std::string text = controlMapText();

    const sgl::ActionId all[] = {
        actions.roll,
        actions.endTurn,
        actions.bid,
        actions.passBid,
        actions.pause,
        actions.help,
        actions.menu.up,
        actions.menu.down,
        actions.menu.confirm,
        actions.menu.cancel,
        actions.menu.left,
        actions.menu.right,
        actions.slot[0],
        actions.slot[1],
        actions.slot[2],
        actions.slot[3],
        actions.slot[4],
        actions.slot[5],
        actions.slot[6],
        actions.slot[7],
        actions.slot[8],
    };

    for(const sgl::ActionId id: all)
    {
        for(const sgl::Key key: map.keysFor(id))
        {
            const std::string_view name = keyName(key);
            ASSERT_FALSE(name.empty()) << "missing keyName for bound key";
            EXPECT_NE(text.find(name), std::string::npos) << name;
        }
    }

    EXPECT_NE(text.find("click"), std::string::npos);
}
