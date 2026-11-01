#include <array>
#include <asteroids/app/Sounds.hpp>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>

namespace sgl::asteroids
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 3> specs{{
        {.key = "fire", .pcm = sgl::blip(980.f, 0.04f, sgl::Waveform::Square, 0.35f)},
        {.key = "hit", .pcm = sgl::blip(420.f, 0.08f, sgl::Waveform::Square, 0.35f)},
        {.key = "die", .pcm = sgl::blip(110.f, 0.35f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::asteroids
