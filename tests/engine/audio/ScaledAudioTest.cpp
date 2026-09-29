#include <gtest/gtest.h>
#include <mocks/AudioSpy.hpp>
#include <sgl/audio/ScaledAudio.hpp>
#include <sgl/audio/SoundId.hpp>

TEST(ScaledAudioShould, scalesPlayVolumeByMaster)
{
    sgl::AudioSpy spy;
    sgl::ScaledAudio audio{spy};
    audio.setMaster(0.5f);
    audio.play(sgl::SoundId{1}, 0.8f);

    ASSERT_EQ(spy.plays.size(), 1u);
    EXPECT_EQ(spy.plays[0].id, (sgl::SoundId{1}));
    EXPECT_FLOAT_EQ(spy.plays[0].volume, 0.4f);
}
