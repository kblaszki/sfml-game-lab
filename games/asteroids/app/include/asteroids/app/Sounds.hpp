#pragma once

#include <sgl/audio/Pcm.hpp>
#include <sgl/audio/SoundId.hpp>
#include <span>
#include <string_view>

namespace sgl::asteroids
{

struct SoundSpec
{
    std::string_view key;
    sgl::Pcm pcm;
};

struct SoundIds
{
    sgl::SoundId fire{};
    sgl::SoundId hit{};
    sgl::SoundId die{};
};

[[nodiscard]] std::span<const SoundSpec> soundSpecs();

} // namespace sgl::asteroids
