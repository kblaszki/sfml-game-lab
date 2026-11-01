#include <sgl/core/Overloaded.hpp>
#include <snake/app/Feedback.hpp>
#include <variant>

namespace sgl::snake
{

Feedback::Feedback(sgl::AudioI& audio, SoundIds sounds)
    : audio_{audio}
    , sounds_{sounds}
{
}

void Feedback::onEvents(std::span<const SnakeEvent> events)
{
    for(const SnakeEvent& event: events)
    {
        std::visit(sgl::Overloaded{
                       [this](const Ate&) { audio_.play(sounds_.eat, 1.f); },
                       [this](const Died&) { audio_.play(sounds_.die, 1.f); },
                       [](const Moved&) {},
                   },
                   event);
    }
}

} // namespace sgl::snake
