#include <array>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>
#include <tower/app/Sounds.hpp>

namespace sgl::tower
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 3> specs{{
        {.key = "jump", .pcm = sgl::blip(740.f, 0.07f, sgl::Waveform::Square, 0.3f)},
        {.key = "land", .pcm = sgl::blip(220.f, 0.05f, sgl::Waveform::Square, 0.28f)},
        {.key = "die", .pcm = sgl::blip(110.f, 0.35f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::tower
