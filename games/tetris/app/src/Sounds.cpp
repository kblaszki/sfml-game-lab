#include <array>
#include <sgl/audio/Blip.hpp>
#include <sgl/audio/Waveform.hpp>
#include <tetris/app/Sounds.hpp>

namespace sgl::tetris
{

std::span<const SoundSpec> soundSpecs()
{
    static const std::array<SoundSpec, 9> specs{{
        {.key = "move", .pcm = sgl::blip(440.f, 0.04f, sgl::Waveform::Square, 0.35f)},
        {.key = "rotate", .pcm = sgl::blip(660.f, 0.05f, sgl::Waveform::Square, 0.35f)},
        {.key = "lock", .pcm = sgl::blip(220.f, 0.08f, sgl::Waveform::Triangle, 0.35f)},
        {.key = "line1", .pcm = sgl::blip(523.f, 0.1f, sgl::Waveform::Square, 0.35f)},
        {.key = "line2", .pcm = sgl::blip(587.f, 0.12f, sgl::Waveform::Square, 0.35f)},
        {.key = "line3", .pcm = sgl::blip(659.f, 0.14f, sgl::Waveform::Square, 0.35f)},
        {.key = "tetris", .pcm = sgl::blip(784.f, 0.22f, sgl::Waveform::Square, 0.35f)},
        {.key = "levelUp", .pcm = sgl::blip(880.f, 0.16f, sgl::Waveform::Triangle, 0.35f)},
        {.key = "gameOver", .pcm = sgl::blip(110.f, 0.35f, sgl::Waveform::Triangle, 0.35f)},
    }};
    return specs;
}

} // namespace sgl::tetris
