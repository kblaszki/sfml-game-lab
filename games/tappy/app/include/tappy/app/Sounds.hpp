#pragma once

#include <sgl/audio/Pcm.hpp>
#include <sgl/audio/SoundId.hpp>
#include <span>
#include <string_view>

namespace sgl::tappy
{

struct SoundSpec
{
    std::string_view key;
    sgl::Pcm pcm;
};

struct SoundIds
{
    sgl::SoundId flap{};
    sgl::SoundId clear{};
    sgl::SoundId die{};
};

[[nodiscard]] std::span<const SoundSpec> soundSpecs();

} // namespace sgl::tappy
