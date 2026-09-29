#pragma once

#include <sgl/audio/AudioI.hpp>
#include <span>
#include <syndicate/app/Sounds.hpp>
#include <syndicate/sim/Events.hpp>

namespace sgl::syndicate
{

class Feedback
{
public:
    Feedback(sgl::AudioI& audio, SoundIds sounds);

    void onEvents(std::span<const GameEvent> events);

private:
    sgl::AudioI& audio_;
    SoundIds sounds_{};
};

} // namespace sgl::syndicate
