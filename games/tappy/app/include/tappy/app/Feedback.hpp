#pragma once

#include <sgl/audio/AudioI.hpp>
#include <span>
#include <tappy/app/Sounds.hpp>
#include <tappy/sim/Events.hpp>

namespace sgl::tappy
{

class Feedback
{
public:
    Feedback(sgl::AudioI& audio, SoundIds sounds);

    void onEvents(std::span<const TappyEvent> events);

private:
    sgl::AudioI& audio_;
    SoundIds sounds_{};
};

} // namespace sgl::tappy
