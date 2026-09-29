#include <engine/scene/fakes/SceneSpy.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <sgl/core/Time.hpp>
#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/ui/MenuActions.hpp>
#include <sgl/ui/PauseOverlay.hpp>

namespace
{

constexpr sgl::ActionId kUp{.id = 1};
constexpr sgl::ActionId kDown{.id = 2};
constexpr sgl::ActionId kConfirm{.id = 3};
constexpr sgl::ActionId kCancel{.id = 4};

constexpr sgl::MenuActions kActions{.up = kUp, .down = kDown, .confirm = kConfirm, .cancel = kCancel};

[[nodiscard]] sgl::ActionMap makeMap()
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Up, kUp);
    map.bind(sgl::Key::Down, kDown);
    map.bind(sgl::Key::Enter, kConfirm);
    map.bind(sgl::Key::Escape, kCancel);
    return map;
}

[[nodiscard]] sgl::SceneFactory unusedPauseFactory()
{
    return [] { return std::make_unique<sgl::SceneSpy>(); };
}

[[nodiscard]] sgl::InputState withKey(sgl::Key key)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, makeMap());
    return input;
}

} // namespace

TEST(PauseOverlayShould, resumeRequestsPopScene)
{
    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::make_unique<sgl::SceneSpy>());
    stack.push(std::make_unique<sgl::PauseOverlay>(kActions, sgl::FontId{.id = 1}));

    EXPECT_EQ(stack.size(), 2u);
    stack.update(withKey(sgl::Key::Enter), sgl::kTick);
    EXPECT_EQ(stack.size(), 1u);
    EXPECT_FALSE(stack.quitRequested());
}

TEST(PauseOverlayShould, quitRequestsQuitApp)
{
    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::make_unique<sgl::SceneSpy>());
    stack.push(std::make_unique<sgl::PauseOverlay>(kActions, sgl::FontId{.id = 1}));

    stack.update(withKey(sgl::Key::Down), sgl::kTick);
    EXPECT_EQ(stack.size(), 2u);

    stack.update(withKey(sgl::Key::Enter), sgl::kTick);
    EXPECT_TRUE(stack.quitRequested());
}

TEST(PauseOverlayShould, cancelResumesWithPopScene)
{
    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::make_unique<sgl::SceneSpy>());
    stack.push(std::make_unique<sgl::PauseOverlay>(kActions, sgl::FontId{.id = 1}));

    stack.update(withKey(sgl::Key::Escape), sgl::kTick);
    EXPECT_EQ(stack.size(), 1u);
    EXPECT_FALSE(stack.quitRequested());
}
