#include <sgl/core/Overloaded.hpp>
#include <tower/app/Feedback.hpp>
#include <variant>

namespace sgl::tower
{

Feedback::Feedback(sgl::AudioI& audio, SoundIds sounds)
    : audio_{audio}
    , sounds_{sounds}
{
}

void Feedback::onEvents(std::span<const TowerEvent> events)
{
    for(const TowerEvent& event: events)
    {
        std::visit(sgl::Overloaded{
                       [this](const Landed&) { audio_.play(sounds_.land, 1.f); },
                       [this](const Died&) { audio_.play(sounds_.die, 1.f); },
                       [this](const WallBounced&) { audio_.play(sounds_.wall, 1.f); },
                       [this](const BonusPicked&) { audio_.play(sounds_.bonus, 1.f); },
                   },
                   event);
    }
}

} // namespace sgl::tower
