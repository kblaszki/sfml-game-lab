#pragma once

#include <sgl/audio/AudioI.hpp>
#include <span>
#include <tower/app/Sounds.hpp>
#include <tower/sim/Events.hpp>

namespace sgl::tower
{

class Feedback
{
public:
    Feedback(sgl::AudioI& audio, SoundIds sounds);

    void onEvents(std::span<const TowerEvent> events);

private:
    sgl::AudioI& audio_;
    SoundIds sounds_{};
};

} // namespace sgl::tower
