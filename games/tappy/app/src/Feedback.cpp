#include <sgl/core/Overloaded.hpp>
#include <tappy/app/Feedback.hpp>
#include <variant>

namespace sgl::tappy
{

Feedback::Feedback(sgl::AudioI& audio, SoundIds sounds)
    : audio_{audio}
    , sounds_{sounds}
{
}

void Feedback::onEvents(std::span<const TappyEvent> events)
{
    for(const TappyEvent& event: events)
    {
        std::visit(sgl::Overloaded{
                       [this](const Flapped&) { audio_.play(sounds_.flap, 1.f); },
                       [this](const GateCleared&) { audio_.play(sounds_.clear, 1.f); },
                       [this](const Died&) { audio_.play(sounds_.die, 1.f); },
                   },
                   event);
    }
}

} // namespace sgl::tappy
