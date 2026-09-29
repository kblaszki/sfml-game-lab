#pragma once

#include <algorithm>
#include <sgl/audio/AudioI.hpp>

namespace sgl
{

class ScaledAudio final : public AudioI
{
public:
    explicit ScaledAudio(AudioI& inner)
        : inner_{inner}
    {
    }

    void setMaster(float volume01) noexcept
    {
        master_ = std::clamp(volume01, 0.f, 1.f);
    }

    void play(SoundId id, float volume) override
    {
        inner_.play(id, volume * master_);
    }

private:
    AudioI& inner_;
    float master_{1.f};
};

} // namespace sgl
