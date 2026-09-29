---
title: Add a custom sgl::ui component
diataxis: how-to
audience: [ai, human]
related_code:
  - engine/ui/CMakeLists.txt
  - engine/ui/include/sgl/ui/MenuModel.hpp
  - engine/ui/include/sgl/ui/RenderMenu.hpp
  - engine/ui/include/sgl/ui/HoverAnim.hpp
  - engine/ui/include/sgl/ui/TooltipSystem.hpp
  - engine/ui/include/sgl/ui/TextLayout.hpp
  - engine/render/include/sgl/render/Shapes.hpp
  - engine/render/include/sgl/render/DrawCommand.hpp
  - engine/anim/include/sgl/anim/Easing.hpp
  - tests/engine/ui/CMakeLists.txt
related_docs:
  - ../reference/engine-ui.md
  - ../reference/engine-render.md
  - ../reference/engine-anim.md
  - write-headless-tests.md
  - ../explanation/sfml-integration.md
keywords: [ui, sgl_ui, widget, component, ProgressBar, MenuModel, HoverAnim, TooltipSystem, Shapes, roundedRect, MeshCmd]
last_reviewed: 2026-09-29
---

# Add a custom sgl::ui component

`sgl_ui` widgets follow one pattern:

- **State** is a small class updated with plain inputs (an `InputState`, a pointer position, `Seconds dt`).
- **Style** is a separate aggregate of colors and sizes with defaults.
- **Drawing** is a `const` function that pushes `DrawCommand`s into a `RenderQueue`.

Nothing in `sgl_ui` includes SFML. `MenuModel` + `renderMenu`, `HoverAnim` and `TooltipSystem` are the existing examples.

Put a component in `engine/ui` only when a second game could use it. A widget used by one game belongs in `games/<game>/app/`, with the same pattern.

The example below adds a `ProgressBar` whose fill eases toward a target value. It has been compiled and tested against the engine.

## 1. Header

`engine/ui/include/sgl/ui/ProgressBar.hpp`:

```cpp
#pragma once

#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>

namespace sgl
{

struct ProgressBarStyle
{
    Color track{40, 44, 60, 255};
    Color fill{90, 200, 255, 255};
    float cornerRadius{4.f};
    float followRate{8.f};
};

class ProgressBar
{
public:
    void setTarget(float value) noexcept;
    void update(Seconds dt, const ProgressBarStyle& style) noexcept;
    [[nodiscard]] float shown() const noexcept;
    void render(RenderQueue& queue, Rect<float> area, const ProgressBarStyle& style, Layer layer) const;

private:
    float target_{};
    float shown_{};
};

} // namespace sgl
```

Give every style field a default, so callers can write `ProgressBarStyle{}` without `-Wmissing-field-initializers` errors.

## 2. Implementation

`engine/ui/src/ProgressBar.cpp`. The procedural shapes in `<sgl/render/Shapes.hpp>` return `MeshCmd`s that the SFML backend draws as one `VertexArray` each:

```cpp
#include <algorithm>
#include <sgl/render/Shapes.hpp>
#include <sgl/ui/ProgressBar.hpp>

namespace sgl
{

void ProgressBar::setTarget(float value) noexcept
{
    target_ = std::clamp(value, 0.f, 1.f);
}

void ProgressBar::update(Seconds dt, const ProgressBarStyle& style) noexcept
{
    const float t = std::min(1.f, style.followRate * dt.count());
    shown_ += (target_ - shown_) * t;
}

float ProgressBar::shown() const noexcept
{
    return shown_;
}

void ProgressBar::render(RenderQueue& queue, Rect<float> area, const ProgressBarStyle& style, Layer layer) const
{
    queue.push(layer, 0.f, roundedRect(area, style.cornerRadius, style.track));
    if(shown_ <= 0.f)
    {
        return;
    }
    const Rect<float> filled{area.pos, {area.size.x * shown_, area.size.y}};
    queue.push(layer, 1.f, roundedRect(filled, style.cornerRadius, style.fill));
}

} // namespace sgl
```

Use `depth` to order the parts of one widget within a layer, rather than relying on the order of the `push` calls. For nonlinear motion use `sgl_anim` (`Easing`, `Tween`) instead of hand-written curves.

## 3. Wire it into CMake

Add `src/ProgressBar.cpp` to the `add_library(sgl_ui STATIC ...)` list in [engine/ui/CMakeLists.txt](../../engine/ui/CMakeLists.txt). If the component needs a module `sgl_ui` does not link yet, add it to `target_link_libraries(sgl_ui ...)`, and never add SFML.

## 4. Test it without a window

Add `tests/engine/ui/ProgressBarTest.cpp` and register it in [tests/engine/ui/CMakeLists.txt](../../tests/engine/ui/CMakeLists.txt):

```cmake
add_unit_test(progress_bar_test
    SOURCES
        ProgressBarTest.cpp
    DEPENDENCIES
        sgl_ui
)
```

```cpp
#include <gtest/gtest.h>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/ProgressBar.hpp>

TEST(ProgressBarShould, easeTowardTarget)
{
    sgl::ProgressBar bar;
    const sgl::ProgressBarStyle style{};
    bar.setTarget(1.f);

    bar.update(sgl::Seconds{0.05f}, style);

    EXPECT_GT(bar.shown(), 0.f);
    EXPECT_LT(bar.shown(), 1.f);
}

TEST(ProgressBarShould, drawTrackThenFill)
{
    sgl::ProgressBar bar;
    const sgl::ProgressBarStyle style{};
    bar.setTarget(0.5f);
    bar.update(sgl::Seconds{1.f}, style);
    sgl::RenderQueue queue;

    bar.render(queue, {{0.f, 0.f}, {200.f, 16.f}}, style, sgl::Layer::Hud);

    const auto entries = queue.sorted();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_LT(entries[0].key.depth, entries[1].key.depth);
    EXPECT_TRUE(std::holds_alternative<sgl::MeshCmd>(entries[1].command));
}
```

Assert on the commands (count, layer, depth, variant alternative, rect), not on pixels.

## 5. Use it from a scene

Keep the widget as a member of the scene. Call `update` from `SceneI::update` with the scene's `dt` (always `kTick`), and `render` from `SceneI::render`. For hover effects, pass `ctx.input().pointer()` into the widget, as `TooltipSystem::update` does. For text that must wrap or fit, measure it with `TextLayout` rather than guessing widths.

## 6. Update docs

Add the component to [engine-ui.md](../reference/engine-ui.md) (API, style defaults, `related_code`), and add the new source and suite to [source-layout.md](../reference/source-layout.md).
