#include <array>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>
#include <snake/app/Sounds.hpp>

namespace sgl::snake
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 2> specs{{
        {.key = "eat", .pcm = sgl::blip(740.f, 0.07f, sgl::Waveform::Square, 0.35f)},
        {.key = "die", .pcm = sgl::blip(110.f, 0.35f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::snake
