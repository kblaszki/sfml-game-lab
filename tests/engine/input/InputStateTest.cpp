#include <gtest/gtest.h>
#include <sgl/core/Vec2.hpp>
#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/ActionState.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/input/MouseButton.hpp>

namespace
{

constexpr sgl::ActionId kJump{.id = 1};
constexpr sgl::ActionId kFire{.id = 2};
constexpr sgl::AxisId kMoveX{.id = 10};

} // namespace

TEST(InputStateTest, EdgeLastsOneFrame)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Space, kJump);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::Space}, map);

    const sgl::ActionState pressed = state.action(kJump);
    EXPECT_TRUE(pressed.pressed);
    EXPECT_TRUE(pressed.held);
    EXPECT_FALSE(pressed.released);

    state.beginFrame();
    const sgl::ActionState heldOnly = state.action(kJump);
    EXPECT_FALSE(heldOnly.pressed);
    EXPECT_TRUE(heldOnly.held);
    EXPECT_FALSE(heldOnly.released);
}

TEST(InputStateTest, KeyDownThenKeyUpSameFrameYieldsPressedAndReleased)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Space, kJump);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::Space}, map);
    state.apply(sgl::KeyUp{.key = sgl::Key::Space}, map);

    const sgl::ActionState sameFrame = state.action(kJump);
    EXPECT_TRUE(sameFrame.pressed);
    EXPECT_TRUE(sameFrame.released);
    EXPECT_FALSE(sameFrame.held);
}

TEST(InputStateTest, HoldPersistsAcrossBeginFrame)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::A, kFire);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::A}, map);
    state.beginFrame();
    state.beginFrame();

    EXPECT_TRUE(state.action(kFire).held);
    EXPECT_FALSE(state.action(kFire).pressed);
    EXPECT_FALSE(state.action(kFire).released);
}

TEST(InputStateTest, BothAxisKeysYieldZero)
{
    sgl::ActionMap map;
    map.bindAxis(sgl::Key::Left, sgl::Key::Right, kMoveX);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::Left}, map);
    state.apply(sgl::KeyDown{.key = sgl::Key::Right}, map);

    EXPECT_FLOAT_EQ(state.axis(kMoveX), 0.f);
}

TEST(InputStateTest, OneAxisKeyYieldsSignedValue)
{
    sgl::ActionMap map;
    map.bindAxis(sgl::Key::Left, sgl::Key::Right, kMoveX);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::Left}, map);
    EXPECT_FLOAT_EQ(state.axis(kMoveX), -1.f);

    state.apply(sgl::KeyUp{.key = sgl::Key::Left}, map);
    state.apply(sgl::KeyDown{.key = sgl::Key::Right}, map);
    EXPECT_FLOAT_EQ(state.axis(kMoveX), 1.f);
}

TEST(InputStateTest, FocusLostClearsHold)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::D, kFire);
    map.bindAxis(sgl::Key::Left, sgl::Key::Right, kMoveX);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::D}, map);
    state.apply(sgl::KeyDown{.key = sgl::Key::Right}, map);
    EXPECT_TRUE(state.action(kFire).held);
    EXPECT_FLOAT_EQ(state.axis(kMoveX), 1.f);

    state.apply(sgl::FocusLost{}, map);
    EXPECT_TRUE(state.focusLost());
    EXPECT_FALSE(state.action(kFire).held);
    EXPECT_FLOAT_EQ(state.axis(kMoveX), 0.f);
}

TEST(InputStateTest, UnboundKeyIsNoOp)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::A, kJump);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::Escape}, map);

    EXPECT_FALSE(state.action(kJump).pressed);
    EXPECT_FALSE(state.action(kJump).held);
    EXPECT_FALSE(state.closeRequested());
    EXPECT_FALSE(state.focusLost());
}

TEST(InputStateTest, MouseDownSetsPointer)
{
    sgl::ActionMap map;
    sgl::InputState state;

    state.apply(sgl::MouseDown{.button = sgl::MouseButton::Left, .pos = sgl::Vec2f{.x = 12.f, .y = 34.f}}, map);

    ASSERT_TRUE(state.pointer().has_value());
    EXPECT_FLOAT_EQ(state.pointer()->x, 12.f);
    EXPECT_FLOAT_EQ(state.pointer()->y, 34.f);
}

TEST(InputStateTest, CloseRequestedOnWindowClosed)
{
    sgl::ActionMap map;
    sgl::InputState state;

    state.apply(sgl::WindowClosed{}, map);
    EXPECT_TRUE(state.closeRequested());

    state.beginFrame();
    EXPECT_FALSE(state.closeRequested());
}

TEST(ActionMapTest, OneKeyMapsToSeveralActions)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Enter, kJump);
    map.bind(sgl::Key::Enter, kFire);

    const auto actions = map.actionsFor(sgl::Key::Enter);
    ASSERT_EQ(actions.size(), 2u);
    EXPECT_EQ(actions[0], kJump);
    EXPECT_EQ(actions[1], kFire);
}

TEST(InputStateTest, ReleasedEdgeLastsOneFrame)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Space, kJump);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::Space}, map);
    state.beginFrame();
    state.apply(sgl::KeyUp{.key = sgl::Key::Space}, map);

    const sgl::ActionState released = state.action(kJump);
    EXPECT_TRUE(released.released);
    EXPECT_FALSE(released.held);
    EXPECT_FALSE(released.pressed);

    state.beginFrame();
    const sgl::ActionState after = state.action(kJump);
    EXPECT_FALSE(after.released);
    EXPECT_FALSE(after.held);
    EXPECT_FALSE(after.pressed);
}

TEST(InputStateTest, HoldSurvivesReleaseOfOneOfTwoKeys)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::A, kJump);
    map.bind(sgl::Key::D, kJump);

    sgl::InputState state;
    state.apply(sgl::KeyDown{.key = sgl::Key::A}, map);
    state.apply(sgl::KeyDown{.key = sgl::Key::D}, map);
    state.beginFrame();
    state.apply(sgl::KeyUp{.key = sgl::Key::A}, map);

    const sgl::ActionState stillHeld = state.action(kJump);
    EXPECT_TRUE(stillHeld.held);
    EXPECT_FALSE(stillHeld.released);
    EXPECT_FALSE(stillHeld.pressed);

    state.beginFrame();
    state.apply(sgl::KeyUp{.key = sgl::Key::D}, map);

    const sgl::ActionState fullyReleased = state.action(kJump);
    EXPECT_FALSE(fullyReleased.held);
    EXPECT_TRUE(fullyReleased.released);
}

TEST(InputStateTest, PointerPressedEdge)
{
    sgl::ActionMap map;
    sgl::InputState state;

    state.apply(sgl::MouseDown{.button = sgl::MouseButton::Left, .pos = sgl::Vec2f{.x = 1.f, .y = 2.f}}, map);
    EXPECT_TRUE(state.pointerPressed());
    EXPECT_FALSE(state.pointerReleased());
    ASSERT_TRUE(state.pointer().has_value());
    EXPECT_FLOAT_EQ(state.pointer()->x, 1.f);
    EXPECT_FLOAT_EQ(state.pointer()->y, 2.f);

    state.beginFrame();
    EXPECT_FALSE(state.pointerPressed());
    EXPECT_FALSE(state.pointerReleased());

    state.apply(sgl::MouseUp{.button = sgl::MouseButton::Left, .pos = sgl::Vec2f{.x = 3.f, .y = 4.f}}, map);
    EXPECT_TRUE(state.pointerReleased());
    EXPECT_FALSE(state.pointerPressed());
    ASSERT_TRUE(state.pointer().has_value());
    EXPECT_FLOAT_EQ(state.pointer()->x, 3.f);
    EXPECT_FLOAT_EQ(state.pointer()->y, 4.f);

    state.beginFrame();
    EXPECT_FALSE(state.pointerReleased());
}
