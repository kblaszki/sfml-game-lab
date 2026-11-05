#pragma once

#include <sgl/audio/Pcm.hpp>
#include <sgl/audio/SoundId.hpp>
#include <span>
#include <string_view>

namespace sgl::tower
{

struct SoundSpec
{
    std::string_view key;
    sgl::Pcm pcm;
};

struct SoundIds
{
    sgl::SoundId jump{};
    sgl::SoundId land{};
    sgl::SoundId die{};
    sgl::SoundId wall{};
    sgl::SoundId bonus{};
};

[[nodiscard]] std::span<const SoundSpec> soundSpecs();

} // namespace sgl::tower
