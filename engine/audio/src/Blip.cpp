#include <cstdint>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Envelope.hpp>
#include <sgl/audio/Tone.hpp>
#include <sgl/core/Time.hpp>

namespace sgl
{

Pcm blip(float hz, float seconds, Waveform waveform, float gain)
{
    constexpr std::uint32_t sampleRate{22050};
    return tone(waveform,
                hz,
                Seconds{seconds},
                sampleRate,
                Envelope{.attack = Seconds{0.005f}, .release = Seconds{0.03f}},
                gain);
}

} // namespace sgl
