#include <arkanoid/app/Sounds.hpp>
#include <array>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>

namespace sgl::arkanoid
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 9> specs{{
        {.key = "paddle", .pcm = sgl::blip(330.f, 0.05f, sgl::Waveform::Square, 0.35f)},
        {.key = "brick", .pcm = sgl::blip(520.f, 0.06f, sgl::Waveform::Square, 0.35f)},
        {.key = "wall", .pcm = sgl::blip(220.f, 0.04f, sgl::Waveform::Triangle, 0.35f)},
        {.key = "launch", .pcm = sgl::blip(440.f, 0.08f, sgl::Waveform::Square, 0.35f)},
        {.key = "powerUp", .pcm = sgl::blip(660.f, 0.12f, sgl::Waveform::Triangle, 0.35f)},
        {.key = "ballLost", .pcm = sgl::blip(180.f, 0.1f, sgl::Waveform::Triangle, 0.35f)},
        {.key = "lifeLost", .pcm = sgl::blip(140.f, 0.18f, sgl::Waveform::Triangle, 0.35f)},
        {.key = "stageClear", .pcm = sgl::blip(784.f, 0.22f, sgl::Waveform::Square, 0.35f)},
        {.key = "gameOver", .pcm = sgl::blip(110.f, 0.35f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::arkanoid
