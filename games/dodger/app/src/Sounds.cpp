#include <array>
#include <dodger/app/Sounds.hpp>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>

namespace sgl::dodger
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 1> specs{{
        {.key = "die", .pcm = sgl::blip(110.f, 0.35f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::dodger
