#include <gtest/gtest.h>
#include <sgl/core/Rect.hpp>
#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/input/MouseButton.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <vector>

namespace
{

constexpr sgl::ActionId kUp{.id = 1};
constexpr sgl::ActionId kDown{.id = 2};
constexpr sgl::ActionId kConfirm{.id = 3};
constexpr sgl::ActionId kCancel{.id = 4};

constexpr sgl::MenuActions kActions{.up = kUp, .down = kDown, .confirm = kConfirm, .cancel = kCancel, .left = {}, .right = {}};

[[nodiscard]] sgl::ActionMap makeMap()
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Up, kUp);
    map.bind(sgl::Key::Down, kDown);
    map.bind(sgl::Key::Enter, kConfirm);
    map.bind(sgl::Key::Escape, kCancel);
    return map;
}

[[nodiscard]] std::vector<sgl::Button> twoButtons()
{
    return {
        sgl::Button{.rect = {.pos = {0.f, 0.f}, .size = {100.f, 40.f}}, .label = "A"},
        sgl::Button{.rect = {.pos = {0.f, 50.f}, .size = {100.f, 40.f}}, .label = "B"},
    };
}

[[nodiscard]] sgl::InputState withKey(sgl::Key key)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, makeMap());
    return input;
}

} // namespace

TEST(MenuModelShould, wrapKeyboardFocus)
{
    sgl::MenuModel model{twoButtons()};
    EXPECT_EQ(model.focus(), 0u);

    EXPECT_FALSE(model.update(withKey(sgl::Key::Down), kActions).has_value());
    EXPECT_EQ(model.focus(), 1u);

    EXPECT_FALSE(model.update(withKey(sgl::Key::Down), kActions).has_value());
    EXPECT_EQ(model.focus(), 0u);

    EXPECT_FALSE(model.update(withKey(sgl::Key::Up), kActions).has_value());
    EXPECT_EQ(model.focus(), 1u);
}

TEST(MenuModelShould, pointerHoverChangesFocus)
{
    sgl::MenuModel model{twoButtons()};
    sgl::ActionMap map;
    sgl::InputState input;
    input.apply(sgl::MouseMove{.pos = {10.f, 60.f}}, map);

    EXPECT_FALSE(model.update(input, kActions).has_value());
    EXPECT_EQ(model.focus(), 1u);
}

TEST(MenuModelShould, clickPressAndReleaseInsideActivates)
{
    sgl::MenuModel model{twoButtons()};
    sgl::ActionMap map;

    sgl::InputState press;
    press.apply(sgl::MouseDown{.button = sgl::MouseButton::Left, .pos = {10.f, 10.f}}, map);
    EXPECT_FALSE(model.update(press, kActions).has_value());

    press.beginFrame();
    press.apply(sgl::MouseUp{.button = sgl::MouseButton::Left, .pos = {20.f, 20.f}}, map);
    const auto activated = model.update(press, kActions);
    ASSERT_TRUE(activated.has_value());
    EXPECT_EQ(*activated, 0u);
}

TEST(MenuModelShould, pressInsideReleaseOutsideDoesNotActivate)
{
    sgl::MenuModel model{twoButtons()};
    sgl::ActionMap map;

    sgl::InputState input;
    input.apply(sgl::MouseDown{.button = sgl::MouseButton::Left, .pos = {10.f, 10.f}}, map);
    EXPECT_FALSE(model.update(input, kActions).has_value());

    input.beginFrame();
    input.apply(sgl::MouseUp{.button = sgl::MouseButton::Left, .pos = {200.f, 200.f}}, map);
    EXPECT_FALSE(model.update(input, kActions).has_value());
}

TEST(MenuModelShould, cancelSetsCancelled)
{
    sgl::MenuModel model{twoButtons()};
    EXPECT_FALSE(model.cancelled());

    EXPECT_FALSE(model.update(withKey(sgl::Key::Escape), kActions).has_value());
    EXPECT_TRUE(model.cancelled());
}

TEST(MenuModelShould, confirmActivatesFocus)
{
    sgl::MenuModel model{twoButtons()};
    EXPECT_FALSE(model.update(withKey(sgl::Key::Down), kActions).has_value());

    const auto activated = model.update(withKey(sgl::Key::Enter), kActions);
    ASSERT_TRUE(activated.has_value());
    EXPECT_EQ(*activated, 1u);
}
