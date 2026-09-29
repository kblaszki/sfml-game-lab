#include <cmath>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/HandView.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/Cards.hpp>
#include <variant>

using sgl::syndicate::CardKind;
using sgl::syndicate::Hand;
using sgl::syndicate::HandView;
using sgl::syndicate::defaultTheme;

namespace
{

[[nodiscard]] float firstCardX(sgl::RenderQueue& queue)
{
    for(const sgl::Entry& e: queue.sorted())
    {
        if(const auto* mesh = std::get_if<sgl::MeshCmd>(&e.command))
        {
            if(mesh->vertices.empty())
            {
                continue;
            }
            float minX = mesh->vertices.front().position.x;
            for(const sgl::Vertex& v: mesh->vertices)
            {
                minX = std::min(minX, v.position.x);
            }
            return minX;
        }
    }
    return 0.f;
}

} // namespace

TEST(HandViewTest, slideReachesSlotAfterAppearDuration)
{
    Hand hand;
    ASSERT_TRUE(hand.add(CardKind::Subsidy).has_value());

    HandView view;
    const auto theme = defaultTheme();
    const sgl::FontId font{.id = 1};

    view.sync(hand, sgl::Seconds{0.f});
    {
        sgl::RenderQueue queue;
        view.render(queue, font, theme);
        const float x0 = firstCardX(queue);
        EXPECT_LT(x0, 24.f);
    }

    view.sync(hand, sgl::Seconds{0.35f});
    {
        sgl::RenderQueue queue;
        view.render(queue, font, theme);
        const float x = firstCardX(queue);
        EXPECT_NEAR(x, 24.f, 1.f);
    }
}
