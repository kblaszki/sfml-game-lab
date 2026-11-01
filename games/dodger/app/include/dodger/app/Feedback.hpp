#pragma once

#include <dodger/app/Sounds.hpp>
#include <dodger/sim/Events.hpp>
#include <sgl/audio/AudioI.hpp>
#include <span>

namespace sgl::dodger
{

class Feedback
{
public:
    Feedback(sgl::AudioI& audio, SoundIds sounds);

    void onEvents(std::span<const DodgerEvent> events);

private:
    sgl::AudioI& audio_;
    SoundIds sounds_{};
};

} // namespace sgl::dodger
