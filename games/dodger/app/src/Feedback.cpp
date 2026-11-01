#include <dodger/app/Feedback.hpp>
#include <sgl/core/Overloaded.hpp>
#include <variant>

namespace sgl::dodger
{

Feedback::Feedback(sgl::AudioI& audio, SoundIds sounds)
    : audio_{audio}
    , sounds_{sounds}
{
}

void Feedback::onEvents(std::span<const DodgerEvent> events)
{
    for(const DodgerEvent& event: events)
    {
        std::visit(sgl::Overloaded{
                       [](const HazardCleared&) {},
                       [this](const Died&) { audio_.play(sounds_.die, 1.f); },
                   },
                   event);
    }
}

} // namespace sgl::dodger
