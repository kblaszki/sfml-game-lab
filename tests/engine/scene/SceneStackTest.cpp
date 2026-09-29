#include <engine/scene/fakes/SceneSpy.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneStack.hpp>

namespace
{

sgl::SceneFactory pauseOverlayFactory()
{
    return [] {
        auto overlay = std::make_unique<sgl::SceneSpy>();
        overlay->traits_ = sgl::SceneTraits{.opaque = false, .blocksUpdate = true, .pausable = false};
        return overlay;
    };
}

sgl::SceneSpy& pushSpy(sgl::SceneStack& stack, sgl::SceneTraits traits = {})
{
    auto spy = std::make_unique<sgl::SceneSpy>();
    spy->traits_ = traits;
    sgl::SceneSpy& ref = *spy;
    stack.push(std::move(spy));
    return ref;
}

sgl::InputState makeInput(bool focusLost = false)
{
    sgl::InputState input;
    if(focusLost)
    {
        sgl::ActionMap map;
        input.apply(sgl::FocusLost{}, map);
    }
    return input;
}

class RenderCountingScene : public sgl::SceneI
{
public:
    void update(sgl::SceneContext&, sgl::Seconds) override
    {
        ++updateCount;
    }

    void render(sgl::RenderQueue&) const override
    {
        ++renderCount;
    }

    [[nodiscard]] sgl::SceneTraits traits() const override
    {
        return traits_;
    }

    sgl::SceneTraits traits_{};
    std::uint32_t updateCount{};
    mutable std::uint32_t renderCount{};
};

RenderCountingScene& pushRenderSpy(sgl::SceneStack& stack, sgl::SceneTraits traits)
{
    auto scene = std::make_unique<RenderCountingScene>();
    scene->traits_ = traits;
    RenderCountingScene& ref = *scene;
    stack.push(std::move(scene));
    return ref;
}

} // namespace

TEST(SceneStackShould, applyDeferredRequestsAfterWalk)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& below = pushSpy(stack, {.opaque = true, .blocksUpdate = false, .pausable = false});
    sgl::SceneSpy& top = pushSpy(stack, {.opaque = false, .blocksUpdate = true, .pausable = false});

    std::uint32_t topUpdatesWhileAlive{};
    top.onUpdate = [&](sgl::SceneContext& ctx) {
        topUpdatesWhileAlive = top.updateCount;
        ctx.request(sgl::PopScene{});
    };

    EXPECT_EQ(stack.size(), 2u);
    stack.update(makeInput(), sgl::kTick);

    EXPECT_EQ(topUpdatesWhileAlive, 1u);
    EXPECT_EQ(below.updateCount, 0u);
    EXPECT_EQ(stack.size(), 1u);
}

TEST(SceneStackShould, replaceOnEmptyActsAsPush)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& spy = pushSpy(stack);

    spy.onUpdate = [](sgl::SceneContext& ctx) {
        ctx.request(sgl::PopScene{});
        ctx.request(sgl::ReplaceScene{[] { return std::make_unique<sgl::SceneSpy>(); }});
    };

    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(stack.size(), 1u);
    EXPECT_FALSE(stack.empty());
}

TEST(SceneStackShould, ignorePauseWhenNotPausable)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& spy = pushSpy(stack, {.opaque = true, .blocksUpdate = true, .pausable = false});
    spy.onUpdate = [](sgl::SceneContext& ctx) { ctx.request(sgl::RequestPause{}); };

    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(stack.size(), 1u);
}

TEST(SceneStackShould, ignoreDoublePauseInOneUpdate)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& spy = pushSpy(stack, {.opaque = true, .blocksUpdate = true, .pausable = true});
    spy.onUpdate = [](sgl::SceneContext& ctx) {
        ctx.request(sgl::RequestPause{});
        ctx.request(sgl::RequestPause{});
    };

    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(stack.size(), 2u);
}

TEST(SceneStackShould, setQuitRequestedOnQuitApp)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& spy = pushSpy(stack);
    spy.onUpdate = [](sgl::SceneContext& ctx) { ctx.request(sgl::QuitApp{}); };

    EXPECT_FALSE(stack.quitRequested());
    stack.update(makeInput(), sgl::kTick);
    EXPECT_TRUE(stack.quitRequested());
}

TEST(SceneStackShould, pushPauseOverlayOnFocusLost)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    pushSpy(stack, {.opaque = true, .blocksUpdate = true, .pausable = true});

    stack.update(makeInput(true), sgl::kTick);
    EXPECT_EQ(stack.size(), 2u);
}

TEST(SceneStackShould, applyDeferredPush)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& top = pushSpy(stack);

    top.onUpdate = [](sgl::SceneContext& ctx) {
        ctx.request(sgl::PushScene{[] { return std::make_unique<sgl::SceneSpy>(); }});
    };

    EXPECT_EQ(stack.size(), 1u);
    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(stack.size(), 2u);
}

TEST(SceneStackShould, renderFromTopmostOpaqueUpward)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    RenderCountingScene& below = pushRenderSpy(stack, {.opaque = true, .blocksUpdate = false, .pausable = false});
    RenderCountingScene& mid = pushRenderSpy(stack, {.opaque = true, .blocksUpdate = false, .pausable = false});
    RenderCountingScene& top = pushRenderSpy(stack, {.opaque = false, .blocksUpdate = false, .pausable = false});

    sgl::RenderQueue queue;
    stack.render(queue);

    EXPECT_EQ(below.renderCount, 0u);
    EXPECT_EQ(mid.renderCount, 1u);
    EXPECT_EQ(top.renderCount, 1u);
}

TEST(SceneStackShould, updateOnEmptyStackIsNoOp)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    EXPECT_TRUE(stack.empty());
    stack.update(makeInput(), sgl::kTick);
    EXPECT_TRUE(stack.empty());
    EXPECT_FALSE(stack.quitRequested());
}

TEST(SceneStackShould, ignoreRequestsFromSceneBelowBlocker)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& below = pushSpy(stack, {.opaque = true, .blocksUpdate = false, .pausable = false});
    sgl::SceneSpy& blocker = pushSpy(stack, {.opaque = false, .blocksUpdate = true, .pausable = false});

    below.onUpdate = [](sgl::SceneContext& ctx) {
        ctx.request(sgl::PushScene{[] { return std::make_unique<sgl::SceneSpy>(); }});
    };

    stack.update(makeInput(), sgl::kTick);

    EXPECT_EQ(blocker.updateCount, 1u);
    EXPECT_EQ(below.updateCount, 0u);
    EXPECT_EQ(stack.size(), 2u);
}

TEST(SceneStackShould, resumeAfterPausePop)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& gameplay = pushSpy(stack, {.opaque = true, .blocksUpdate = true, .pausable = true});
    gameplay.onUpdate = [](sgl::SceneContext& ctx) { ctx.request(sgl::RequestPause{}); };

    stack.update(makeInput(), sgl::kTick);
    ASSERT_EQ(stack.size(), 2u);
    EXPECT_EQ(gameplay.updateCount, 1u);

    sgl::SceneSpy* overlay = dynamic_cast<sgl::SceneSpy*>(stack.top());
    ASSERT_NE(overlay, nullptr);
    overlay->onUpdate = [](sgl::SceneContext& ctx) { ctx.request(sgl::PopScene{}); };

    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(stack.size(), 1u);
    EXPECT_EQ(stack.top(), &gameplay);

    gameplay.onUpdate = {};
    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(gameplay.updateCount, 2u);
}

TEST(SceneStackShould, stopUpdateWalkAtFirstBlockingScene)
{
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& below = pushSpy(stack, {.opaque = true, .blocksUpdate = false, .pausable = false});
    sgl::SceneSpy& mid = pushSpy(stack, {.opaque = false, .blocksUpdate = true, .pausable = false});
    sgl::SceneSpy& top = pushSpy(stack, {.opaque = false, .blocksUpdate = false, .pausable = false});

    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(top.updateCount, 1u);
    EXPECT_EQ(mid.updateCount, 1u);
    EXPECT_EQ(below.updateCount, 0u);
}

TEST(SceneStackShould, pushThenPopInSameUpdateLeavesStackUnchanged)
{
    // Requests apply in enqueue order: push then pop removes the scene just pushed.
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& top = pushSpy(stack);

    top.onUpdate = [](sgl::SceneContext& ctx) {
        ctx.request(sgl::PushScene{[] { return std::make_unique<sgl::SceneSpy>(); }});
        ctx.request(sgl::PopScene{});
    };

    EXPECT_EQ(stack.size(), 1u);
    stack.update(makeInput(), sgl::kTick);
    EXPECT_EQ(stack.size(), 1u);
    EXPECT_EQ(stack.top(), &top);
}

TEST(SceneStackShould, popOnEmptyStackIsNoOp)
{
    // Two Pops in one apply walk: first empties the stack, second must not crash.
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& top = pushSpy(stack);

    top.onUpdate = [](sgl::SceneContext& ctx) {
        ctx.request(sgl::PopScene{});
        ctx.request(sgl::PopScene{});
    };

    stack.update(makeInput(), sgl::kTick);
    EXPECT_TRUE(stack.empty());
    EXPECT_FALSE(stack.quitRequested());
}

TEST(SceneStackShould, quitWithPushSameFrameSetsQuitRequested)
{
    // QuitApp only sets quitWanted_; it does not clear the stack. App exits on the flag.
    sgl::SceneStack stack{pauseOverlayFactory()};
    sgl::SceneSpy& top = pushSpy(stack);

    top.onUpdate = [](sgl::SceneContext& ctx) {
        ctx.request(sgl::PushScene{[] { return std::make_unique<sgl::SceneSpy>(); }});
        ctx.request(sgl::QuitApp{});
    };

    stack.update(makeInput(), sgl::kTick);
    EXPECT_TRUE(stack.quitRequested());
    EXPECT_EQ(stack.size(), 2u);
}
