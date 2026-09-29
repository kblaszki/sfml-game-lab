#pragma once

#include <sgl/audio/Pcm.hpp>
#include <sgl/audio/Waveform.hpp>

namespace sgl
{

[[nodiscard]] Pcm blip(float hz, float seconds, Waveform waveform, float gain);

} // namespace sgl
