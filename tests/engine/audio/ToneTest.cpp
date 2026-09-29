#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>
#include <sgl/audio/NullAudio.hpp>
#include <sgl/audio/Tone.hpp>

using sgl::Envelope;
using sgl::NullAudio;
using sgl::Pcm;
using sgl::Seconds;
using sgl::SoundId;
using sgl::tone;
using sgl::Waveform;

TEST(ToneTest, lengthMatchesDuration)
{
    constexpr std::uint32_t sampleRate = 44100;
    const Seconds duration{0.25f};
    const Pcm pcm = tone(Waveform::Square, 440.f, duration, sampleRate, Envelope{}, 0.5f);

    EXPECT_EQ(pcm.sampleRate, sampleRate);
    EXPECT_EQ(pcm.samples.size(), static_cast<std::size_t>(duration.count() * static_cast<float>(sampleRate)));
}

TEST(ToneTest, squareWaveAlternatesSign)
{
    constexpr std::uint32_t sampleRate = 8;
    const Pcm pcm = tone(Waveform::Square, 1.f, Seconds{1.f}, sampleRate, Envelope{}, 1.f);

    ASSERT_EQ(pcm.samples.size(), 8u);
    // One cycle per second at 8 Hz sample rate: 4 samples high, 4 low.
    EXPECT_GT(pcm.samples[0], 0);
    EXPECT_GT(pcm.samples[1], 0);
    EXPECT_GT(pcm.samples[2], 0);
    EXPECT_GT(pcm.samples[3], 0);
    EXPECT_LT(pcm.samples[4], 0);
    EXPECT_LT(pcm.samples[5], 0);
    EXPECT_LT(pcm.samples[6], 0);
    EXPECT_LT(pcm.samples[7], 0);
}

TEST(ToneTest, envelopeStartsAndEndsAtZero)
{
    constexpr std::uint32_t sampleRate = 1000;
    const Envelope envelope{.attack = Seconds{0.1f}, .release = Seconds{0.1f}};
    const Pcm pcm = tone(Waveform::Triangle, 220.f, Seconds{0.5f}, sampleRate, envelope, 0.8f);

    ASSERT_FALSE(pcm.samples.empty());
    EXPECT_EQ(pcm.samples.front(), 0);
    EXPECT_EQ(pcm.samples.back(), 0);
}

TEST(ToneTest, peakNeverExceedsAmplitude)
{
    constexpr float amplitude = 0.5f;
    constexpr std::uint32_t sampleRate = 22050;
    const Pcm pcm = tone(Waveform::Square,
                         880.f,
                         Seconds{0.2f},
                         sampleRate,
                         Envelope{.attack = Seconds{0.01f}, .release = Seconds{0.01f}},
                         amplitude);

    const auto peakLimit = static_cast<std::int16_t>(amplitude * 32767.f + 0.5f);
    for(const std::int16_t sample: pcm.samples)
    {
        EXPECT_LE(std::abs(sample), peakLimit);
    }
}

TEST(NullAudioTest, playDoesNotThrow)
{
    NullAudio audio;
    EXPECT_NO_THROW(audio.play(SoundId{}, 0.f));
    EXPECT_NO_THROW(audio.play(SoundId{.id = 42}, 1.f));
}
