#pragma once

#include <sgl/audio/Pcm.hpp>
#include <sgl/audio/SoundId.hpp>
#include <span>
#include <string_view>

namespace sgl::syndicate
{

struct SoundSpec
{
    std::string_view key;
    sgl::Pcm pcm;
};

struct SoundIds
{
    sgl::SoundId roll{};
    sgl::SoundId fee{};
    sgl::SoundId auction{};
    sgl::SoundId card{};
    sgl::SoundId market{};
};

[[nodiscard]] std::span<const SoundSpec> soundSpecs();

} // namespace sgl::syndicate
