#include <gtest/gtest.h>
#include <sgl/audio/Blip.hpp>

TEST(BlipTest, producesNonEmptyPcm)
{
    const sgl::Pcm pcm = sgl::blip(440.f, 0.05f, sgl::Waveform::Square, 0.35f);
    EXPECT_EQ(pcm.sampleRate, 22050u);
    EXPECT_FALSE(pcm.samples.empty());
}

TEST(BlipTest, isDeterministicForSameArgs)
{
    const sgl::Pcm a = sgl::blip(330.f, 0.08f, sgl::Waveform::Triangle, 0.2f);
    const sgl::Pcm b = sgl::blip(330.f, 0.08f, sgl::Waveform::Triangle, 0.2f);
    EXPECT_EQ(a.sampleRate, b.sampleRate);
    EXPECT_EQ(a.samples, b.samples);
}
