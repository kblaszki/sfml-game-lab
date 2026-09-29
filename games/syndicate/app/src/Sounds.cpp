#include <array>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>
#include <syndicate/app/Sounds.hpp>

namespace sgl::syndicate
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 5> specs{{
        {.key = "roll", .pcm = sgl::blip(520.f, 0.06f, sgl::Waveform::Square, 0.35f)},
        {.key = "fee", .pcm = sgl::blip(180.f, 0.1f, sgl::Waveform::Triangle, 0.35f)},
        {.key = "auction", .pcm = sgl::blip(660.f, 0.12f, sgl::Waveform::Square, 0.35f)},
        {.key = "card", .pcm = sgl::blip(880.f, 0.08f, sgl::Waveform::Square, 0.35f)},
        {.key = "market", .pcm = sgl::blip(330.f, 0.14f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::syndicate
