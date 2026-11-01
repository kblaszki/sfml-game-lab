#pragma once

#include <sgl/audio/AudioI.hpp>
#include <snake/app/Sounds.hpp>
#include <snake/sim/Events.hpp>
#include <span>

namespace sgl::snake
{

class Feedback
{
public:
    Feedback(sgl::AudioI& audio, SoundIds sounds);

    void onEvents(std::span<const SnakeEvent> events);

private:
    sgl::AudioI& audio_;
    SoundIds sounds_{};
};

} // namespace sgl::snake
