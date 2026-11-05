#include <array>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>
#include <tappy/app/Sounds.hpp>

namespace sgl::tappy
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 3> specs{{
        {.key = "flap", .pcm = sgl::blip(620.f, 0.05f, sgl::Waveform::Square, 0.28f)},
        {.key = "clear", .pcm = sgl::blip(880.f, 0.07f, sgl::Waveform::Triangle, 0.3f)},
        {.key = "die", .pcm = sgl::blip(110.f, 0.35f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::tappy
