#include <mocks/RendererMock.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>

namespace
{

sgl::RectCmd makeRect(sgl::Color fill)
{
    return sgl::RectCmd{
        .rect = {.pos = {0.f, 0.f}, .size = {1.f, 1.f}},
        .fill = fill,
    };
}

} // namespace

TEST(RenderQueueTest, SortsByLayerThenDepthThenSeq)
{
    sgl::RenderQueue queue;
    queue.push(sgl::Layer::Hud, 0.f, makeRect({1, 0, 0, 255}));
    queue.push(sgl::Layer::Background, 5.f, makeRect({2, 0, 0, 255}));
    queue.push(sgl::Layer::Background, 1.f, makeRect({3, 0, 0, 255}));
    queue.push(sgl::Layer::Background, 1.f, makeRect({4, 0, 0, 255}));

    const auto sorted = queue.sorted();
    ASSERT_EQ(sorted.size(), 4u);
    EXPECT_EQ(sorted[0].key.layer, sgl::Layer::Background);
    EXPECT_FLOAT_EQ(sorted[0].key.depth, 1.f);
    EXPECT_EQ(sorted[0].key.seq, 2u);
    EXPECT_EQ(sorted[1].key.layer, sgl::Layer::Background);
    EXPECT_FLOAT_EQ(sorted[1].key.depth, 1.f);
    EXPECT_EQ(sorted[1].key.seq, 3u);
    EXPECT_EQ(sorted[2].key.layer, sgl::Layer::Background);
    EXPECT_FLOAT_EQ(sorted[2].key.depth, 5.f);
    EXPECT_EQ(sorted[3].key.layer, sgl::Layer::Hud);
}

TEST(RenderQueueTest, StableOnEqualSortKeys)
{
    sgl::RenderQueue queue;
    // Force equal keys by pushing then rewriting is not possible; equal layer+depth
    // with different seq still orders by seq. Stability matters when keys match fully.
    // Build two entries with identical SortKey by using the same layer/depth and verifying
    // relative order of equal-key groups after interleaving other pushes.
    queue.push(sgl::Layer::World, 2.f, makeRect({10, 0, 0, 255}));
    queue.push(sgl::Layer::World, 2.f, makeRect({20, 0, 0, 255}));
    queue.push(sgl::Layer::Actors, 0.f, makeRect({30, 0, 0, 255}));
    queue.push(sgl::Layer::World, 2.f, makeRect({40, 0, 0, 255}));

    const auto sorted = queue.sorted();
    ASSERT_EQ(sorted.size(), 4u);
    // World depth 2 entries keep push order among themselves (seq 0, 1, 3).
    EXPECT_EQ(std::get<sgl::RectCmd>(sorted[0].command).fill.r, 10);
    EXPECT_EQ(std::get<sgl::RectCmd>(sorted[1].command).fill.r, 20);
    EXPECT_EQ(std::get<sgl::RectCmd>(sorted[2].command).fill.r, 40);
    EXPECT_EQ(sorted[3].key.layer, sgl::Layer::Actors);
}

TEST(RenderQueueTest, identicalKeysKeepPushOrder)
{
    sgl::RenderQueue queue;
    queue.push(sgl::Layer::World, 1.f, makeRect({1, 0, 0, 255}));
    queue.push(sgl::Layer::World, 1.f, makeRect({2, 0, 0, 255}));
    queue.push(sgl::Layer::World, 1.f, makeRect({3, 0, 0, 255}));

    const auto sorted = queue.sorted();
    ASSERT_EQ(sorted.size(), 3u);
    EXPECT_EQ(std::get<sgl::RectCmd>(sorted[0].command).fill.r, 1);
    EXPECT_EQ(std::get<sgl::RectCmd>(sorted[1].command).fill.r, 2);
    EXPECT_EQ(std::get<sgl::RectCmd>(sorted[2].command).fill.r, 3);
    EXPECT_EQ(sorted[0].key.seq, 0u);
    EXPECT_EQ(sorted[1].key.seq, 1u);
    EXPECT_EQ(sorted[2].key.seq, 2u);
}

TEST(RenderQueueTest, ClearResetsSeq)
{
    sgl::RenderQueue queue;
    queue.push(sgl::Layer::World, 0.f, makeRect({1, 0, 0, 255}));
    queue.push(sgl::Layer::World, 0.f, makeRect({2, 0, 0, 255}));
    EXPECT_EQ(queue.size(), 2u);
    EXPECT_EQ(queue.sorted()[1].key.seq, 1u);

    queue.clear();
    EXPECT_EQ(queue.size(), 0u);

    queue.push(sgl::Layer::World, 0.f, makeRect({3, 0, 0, 255}));
    const auto sorted = queue.sorted();
    ASSERT_EQ(sorted.size(), 1u);
    EXPECT_EQ(sorted[0].key.seq, 0u);
}

TEST(RenderQueueTest, MixedCommandsStableSortByLayerDepthSeq)
{
    sgl::RenderQueue queue;
    queue.push(sgl::Layer::Hud, 0.f,
              sgl::TextCmd{
                  .font = {},
                  .text = "hud",
                  .size = 12,
                  .position = {0.f, 0.f},
                  .color = {1, 0, 0, 255},
              });
    queue.push(sgl::Layer::World, 1.f,
              sgl::SpriteCmd{
                  .texture = {},
                  .source = {.pos = {0.f, 0.f}, .size = {1.f, 1.f}},
                  .position = {0.f, 0.f},
              });
    queue.push(sgl::Layer::World, 1.f,
              sgl::LineCmd{
                  .from = {0.f, 0.f},
                  .to = {1.f, 0.f},
                  .thickness = 1.f,
                  .color = {2, 0, 0, 255},
              });
    queue.push(sgl::Layer::Background, 0.f, makeRect({3, 0, 0, 255}));
    queue.push(sgl::Layer::World, 1.f,
              sgl::CircleCmd{
                  .center = {0.f, 0.f},
                  .radius = 1.f,
                  .fill = {4, 0, 0, 255},
              });
    queue.push(sgl::Layer::World, 0.f,
              sgl::LineCmd{
                  .from = {0.f, 0.f},
                  .to = {0.f, 1.f},
                  .thickness = 2.f,
                  .color = {5, 0, 0, 255},
              });

    const auto sorted = queue.sorted();
    ASSERT_EQ(sorted.size(), 6u);

    EXPECT_EQ(sorted[0].key.layer, sgl::Layer::Background);
    EXPECT_TRUE(std::holds_alternative<sgl::RectCmd>(sorted[0].command));

    EXPECT_EQ(sorted[1].key.layer, sgl::Layer::World);
    EXPECT_FLOAT_EQ(sorted[1].key.depth, 0.f);
    EXPECT_TRUE(std::holds_alternative<sgl::LineCmd>(sorted[1].command));
    EXPECT_EQ(std::get<sgl::LineCmd>(sorted[1].command).color.r, 5);

    EXPECT_EQ(sorted[2].key.layer, sgl::Layer::World);
    EXPECT_FLOAT_EQ(sorted[2].key.depth, 1.f);
    EXPECT_EQ(sorted[2].key.seq, 1u);
    EXPECT_TRUE(std::holds_alternative<sgl::SpriteCmd>(sorted[2].command));

    EXPECT_EQ(sorted[3].key.layer, sgl::Layer::World);
    EXPECT_FLOAT_EQ(sorted[3].key.depth, 1.f);
    EXPECT_EQ(sorted[3].key.seq, 2u);
    EXPECT_TRUE(std::holds_alternative<sgl::LineCmd>(sorted[3].command));
    EXPECT_EQ(std::get<sgl::LineCmd>(sorted[3].command).color.r, 2);

    EXPECT_EQ(sorted[4].key.layer, sgl::Layer::World);
    EXPECT_FLOAT_EQ(sorted[4].key.depth, 1.f);
    EXPECT_EQ(sorted[4].key.seq, 4u);
    EXPECT_TRUE(std::holds_alternative<sgl::CircleCmd>(sorted[4].command));
    EXPECT_EQ(std::get<sgl::CircleCmd>(sorted[4].command).fill.r, 4);

    EXPECT_EQ(sorted[5].key.layer, sgl::Layer::Hud);
    EXPECT_TRUE(std::holds_alternative<sgl::TextCmd>(sorted[5].command));
}
