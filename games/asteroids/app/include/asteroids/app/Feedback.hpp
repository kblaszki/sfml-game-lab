#pragma once

#include <asteroids/app/Sounds.hpp>
#include <asteroids/sim/Events.hpp>
#include <sgl/audio/AudioI.hpp>
#include <span>

namespace sgl::asteroids
{

class Feedback
{
public:
    Feedback(sgl::AudioI& audio, SoundIds sounds);

    void onEvents(std::span<const AsteroidsEvent> events);

private:
    sgl::AudioI& audio_;
    SoundIds sounds_{};
};

} // namespace sgl::asteroids
