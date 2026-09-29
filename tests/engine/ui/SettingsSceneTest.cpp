#include <engine/scene/fakes/SceneSpy.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <sgl/core/Time.hpp>
#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/save/Settings.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <sgl/ui/MenuActions.hpp>
#include <sgl/ui/SettingsScene.hpp>

namespace
{

constexpr sgl::ActionId kUp{.id = 1};
constexpr sgl::ActionId kDown{.id = 2};
constexpr sgl::ActionId kConfirm{.id = 3};
constexpr sgl::ActionId kCancel{.id = 4};
constexpr sgl::ActionId kLeft{.id = 5};
constexpr sgl::ActionId kRight{.id = 6};

constexpr sgl::MenuActions kActions{.up = kUp,
                                    .down = kDown,
                                    .confirm = kConfirm,
                                    .cancel = kCancel,
                                    .left = kLeft,
                                    .right = kRight};

[[nodiscard]] sgl::ActionMap makeMap()
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Up, kUp);
    map.bind(sgl::Key::Down, kDown);
    map.bind(sgl::Key::Enter, kConfirm);
    map.bind(sgl::Key::Escape, kCancel);
    map.bind(sgl::Key::Left, kLeft);
    map.bind(sgl::Key::Right, kRight);
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

TEST(SettingsSceneShould, rightOnVolumeRaisesAndCallsOnChange)
{
    sgl::Settings settings{.masterVolume = 80};
    int changeCount = 0;
    sgl::Settings last{};
    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::make_unique<sgl::SceneSpy>());
    stack.push(std::make_unique<sgl::SettingsScene>(
        settings, kActions, sgl::FontId{.id = 1}, [&](const sgl::Settings& next) {
            ++changeCount;
            last = next;
        }));

    stack.update(withKey(sgl::Key::Right), sgl::kTick);

    EXPECT_EQ(changeCount, 1);
    EXPECT_EQ(last.masterVolume, 90);
}

TEST(SettingsSceneShould, rightOnWindowRowTogglesMode)
{
    sgl::Settings settings{.masterVolume = 80, .mode = sgl::WindowMode::Windowed};
    int changeCount = 0;
    sgl::Settings last{};
    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::make_unique<sgl::SceneSpy>());
    stack.push(std::make_unique<sgl::SettingsScene>(
        settings, kActions, sgl::FontId{.id = 1}, [&](const sgl::Settings& next) {
            ++changeCount;
            last = next;
        }));

    stack.update(withKey(sgl::Key::Down), sgl::kTick);
    stack.update(withKey(sgl::Key::Right), sgl::kTick);

    EXPECT_EQ(changeCount, 1);
    EXPECT_EQ(last.mode, sgl::WindowMode::Fullscreen);
}

TEST(SettingsSceneShould, cancelRequestsPopScene)
{
    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::make_unique<sgl::SceneSpy>());
    stack.push(std::make_unique<sgl::SettingsScene>(
        sgl::Settings{}, kActions, sgl::FontId{.id = 1}, [](const sgl::Settings&) {}));

    EXPECT_EQ(stack.size(), 2u);
    stack.update(withKey(sgl::Key::Escape), sgl::kTick);
    EXPECT_EQ(stack.size(), 1u);
    EXPECT_FALSE(stack.quitRequested());
}
