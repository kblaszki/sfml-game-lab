#include <sgl/core/Color.hpp>
#include <sgl/core/Overloaded.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <tetris/app/BoardRender.hpp>
#include <tetris/app/Feedback.hpp>
#include <tetris/sim/Grid.hpp>
#include <variant>

namespace sgl::tetris
{

Feedback::Feedback(sgl::AudioI& audio, SoundIds sounds, sgl::Pcg32 seed)
    : audio_{audio}
    , sounds_{sounds}
    , rng_{seed}
    , particles_{kParticleCapacity}
{
}

void Feedback::onEvents(std::span<const TetrisEvent> events)
{
    for(const TetrisEvent& event: events)
    {
        std::visit(sgl::Overloaded{
                       [this](const PieceShifted&) { audio_.play(sounds_.move, 1.f); },
                       [this](const PieceRotated&) { audio_.play(sounds_.rotate, 1.f); },
                       [this](const PieceLocked&) { audio_.play(sounds_.lock, 1.f); },
                       [this](const LinesCleared& cleared) {
                           playLineSound(cleared.count);
                           for(std::uint8_t i = 0; i < cleared.count; ++i)
                           {
                               emitRowBurst(cleared.rows[i]);
                           }
                       },
                       [this](const LevelUp&) { audio_.play(sounds_.levelUp, 1.f); },
                       [this](const GameOver&) { audio_.play(sounds_.gameOver, 1.f); },
                       [](const PieceSpawned&) {},
                       [](const HardDropped&) {},
                       [](const HoldUsed&) {},
                   },
                   event);
    }
}

void Feedback::update(sgl::Seconds dt)
{
    particles_.update(dt);
}

void Feedback::render(sgl::RenderQueue& queue) const
{
    particles_.render(queue, sgl::Layer::Overlay);
}

std::size_t Feedback::particleCount() const noexcept
{
    return particles_.size();
}

void Feedback::playLineSound(std::uint8_t count)
{
    switch(count)
    {
        case 1:
            audio_.play(sounds_.line1, 1.f);
            break;
        case 2:
            audio_.play(sounds_.line2, 1.f);
            break;
        case 3:
            audio_.play(sounds_.line3, 1.f);
            break;
        case 4:
            audio_.play(sounds_.tetris, 1.f);
            break;
        default:
            break;
    }
}

void Feedback::emitRowBurst(std::uint8_t rowIndex)
{
    const Layout layout = defaultLayout();
    const float wellW = layout.cell * static_cast<float>(Grid::kWidth);
    const float originX = layout.wellOrigin.x + wellW * 0.5f;
    const float originY =
        layout.wellOrigin.y
        + layout.cell * (static_cast<float>(Grid::kVisibleHeight - 1 - static_cast<int>(rowIndex)) + 0.5f);

    particles_.emit(
        sgl::BurstParams{
            .origin = {originX, originY},
            .count = kParticlesPerRow,
            .speedMin = 40.f,
            .speedMax = 160.f,
            .angleMinDeg = 200.f,
            .angleMaxDeg = 340.f,
            .lifeMin = 0.35f,
            .lifeMax = 0.7f,
            .size = 4.f,
            .color = sgl::Color{255, 220, 120, 220},
            .gravity = {0.f, 280.f},
        },
        rng_);
}

} // namespace sgl::tetris
