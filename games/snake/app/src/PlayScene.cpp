#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneContext.hpp>
#include <snake/app/InputMapping.hpp>
#include <snake/app/PlayScene.hpp>
#include <snake/app/Theme.hpp>
#include <snake/sim/Events.hpp>
#include <string>
#include <variant>
#include <vector>

namespace sgl::snake
{
namespace
{

[[nodiscard]] sgl::Rect<float> cellRect(int x, int y)
{
    return sgl::Rect<float>{
        .pos = {static_cast<float>(x) * cellSize, static_cast<float>(kRows - 1 - y) * cellSize},
        .size = {cellSize, cellSize},
    };
}

[[nodiscard]] sgl::Rect<float> inset(sgl::Rect<float> rect, float margin)
{
    return sgl::Rect<float>{
        .pos = {rect.pos.x + margin, rect.pos.y + margin},
        .size = {rect.size.x - margin * 2.f, rect.size.y - margin * 2.f},
    };
}

} // namespace

PlayScene::PlayScene(const AppServices& services)
    : services_{services}
    , game_{services.seed}
{
}

void PlayScene::update(sgl::SceneContext& ctx, sgl::Seconds dt)
{
    if(game_.over())
    {
        return;
    }

    const sgl::InputState& input = ctx.input();
    if(input.action(services_.actions.pause).pressed)
    {
        ctx.request(sgl::RequestPause{});
        return;
    }

    const auto events = game_.step(toSimInput(input, services_.actions), dt);
    for(const SnakeEvent& event: events)
    {
        if(std::holds_alternative<Died>(event))
        {
            ctx.request(sgl::ReplaceScene{gameOver(services_, game_.score())});
            return;
        }
    }
}

void PlayScene::render(sgl::RenderQueue& queue) const
{
    float depth = 0.f;
    for(int y = 0; y < kRows; ++y)
    {
        for(int x = 0; x < kCols; ++x)
        {
            const bool alt = ((x + y) % 2) == 0;
            queue.push(sgl::Layer::World,
                       depth,
                       sgl::RectCmd{
                           .rect = cellRect(x, y),
                           .fill = alt ? gridFill : gridFillAlt,
                       });
            depth += 1.f;
        }
    }

    const std::vector<Cell>& body = game_.body();
    for(std::size_t i = body.size(); i > 0; --i)
    {
        const bool head = i == 1;
        const Cell& cell = body[i - 1];
        queue.push(sgl::Layer::World,
                   depth,
                   sgl::RectCmd{
                       .rect = inset(cellRect(cell.x, cell.y), head ? 2.f : 5.f),
                       .fill = head ? headFill : bodyFill,
                   });
        depth += 1.f;
    }

    const Cell food = game_.food();
    if(food.x >= 0 && food.y >= 0 && food.x < kCols && food.y < kRows)
    {
        const sgl::Rect<float> rect = cellRect(food.x, food.y);
        queue.push(sgl::Layer::World,
                   depth,
                   sgl::CircleCmd{
                       .center = {rect.pos.x + cellSize * 0.5f, rect.pos.y + cellSize * 0.5f},
                       .radius = cellSize * 0.28f,
                       .fill = foodFill,
                   });
    }

    queue.push(sgl::Layer::Hud,
               0.f,
               sgl::RectCmd{
                   .rect = {.pos = {12.f, 8.f}, .size = {220.f, 40.f}},
                   .fill = scorePlate,
               });
    queue.push(sgl::Layer::Hud,
               1.f,
               sgl::TextCmd{
                   .font = services_.font,
                   .text = "Score " + std::to_string(game_.score()),
                   .size = 24,
                   .position = scorePos,
                   .color = hudText,
               });
}

sgl::SceneTraits PlayScene::traits() const
{
    return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = !game_.over()};
}

SnakeGame& PlayScene::game()
{
    return game_;
}

const SnakeGame& PlayScene::game() const
{
    return game_;
}

} // namespace sgl::snake
