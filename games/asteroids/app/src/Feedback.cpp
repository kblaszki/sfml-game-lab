#include <asteroids/app/Feedback.hpp>
#include <sgl/core/Overloaded.hpp>
#include <variant>

namespace sgl::asteroids
{

Feedback::Feedback(sgl::AudioI& audio, SoundIds sounds)
    : audio_{audio}
    , sounds_{sounds}
{
}

void Feedback::onEvents(std::span<const AsteroidsEvent> events)
{
    for(const AsteroidsEvent& event: events)
    {
        std::visit(sgl::Overloaded{
                       [this](const ShotFired&) { audio_.play(sounds_.fire, 1.f); },
                       [this](const RockHit&) { audio_.play(sounds_.hit, 1.f); },
                       [this](const Died&) { audio_.play(sounds_.die, 1.f); },
                       [](const WaveCleared&) {},
                   },
                   event);
    }
}

} // namespace sgl::asteroids
