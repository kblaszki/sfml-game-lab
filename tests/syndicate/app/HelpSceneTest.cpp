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
#include <string_view>
#include <syndicate/app/ControlMap.hpp>
#include <syndicate/app/HelpScene.hpp>

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

[[nodiscard]] std::size_t manageTabIndex()
{
    std::size_t index = 0;
    std::string_view last{};
    for(const auto& row: sgl::syndicate::controlMap())
    {
        if(row.phase != last)
        {
            if(row.phase == "Manage")
            {
                return index;
            }
            last = row.phase;
            ++index;
        }
    }
    return 0;
}

} // namespace

TEST(HelpSceneShould, startOnRequestedTab)
{
    const std::size_t manage = manageTabIndex();
    sgl::syndicate::HelpScene scene{manage, kActions, sgl::FontId{.id = 1}};
    EXPECT_EQ(scene.tab(), "Manage");
}

TEST(HelpSceneShould, rightMovesToNextTab)
{
    const std::size_t manage = manageTabIndex();
    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::make_unique<sgl::SceneSpy>());
    auto scene = std::make_unique<sgl::syndicate::HelpScene>(manage, kActions, sgl::FontId{.id = 1});
    auto* raw = scene.get();
    stack.push(std::move(scene));

    EXPECT_EQ(raw->tab(), "Manage");
    stack.update(withKey(sgl::Key::Right), sgl::kTick);
    EXPECT_EQ(raw->tab(), "Global");
}
