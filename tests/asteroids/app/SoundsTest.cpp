#include <asteroids/app/Sounds.hpp>
#include <gtest/gtest.h>
#include <set>
#include <string_view>

TEST(SoundsTest, fireHitAndDieAreNonEmptyAndUnique)
{
    const auto specs = sgl::asteroids::soundSpecs();
    ASSERT_EQ(specs.size(), 3u);

    std::set<std::string_view> keys;
    for(const sgl::asteroids::SoundSpec& spec: specs)
    {
        EXPECT_FALSE(spec.key.empty());
        EXPECT_GT(spec.pcm.sampleRate, 0u);
        EXPECT_FALSE(spec.pcm.samples.empty());
        EXPECT_TRUE(keys.insert(spec.key).second) << spec.key;
    }

    EXPECT_TRUE(keys.contains("fire"));
    EXPECT_TRUE(keys.contains("hit"));
    EXPECT_TRUE(keys.contains("die"));
}
