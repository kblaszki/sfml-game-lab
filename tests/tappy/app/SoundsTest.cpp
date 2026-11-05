#include <gtest/gtest.h>
#include <set>
#include <string_view>
#include <tappy/app/Sounds.hpp>

TEST(SoundsTest, flapClearDieAreNonEmptyAndUnique)
{
    const auto specs = sgl::tappy::soundSpecs();
    ASSERT_EQ(specs.size(), 3u);

    std::set<std::string_view> keys;
    for(const sgl::tappy::SoundSpec& spec: specs)
    {
        EXPECT_FALSE(spec.key.empty());
        EXPECT_GT(spec.pcm.sampleRate, 0u);
        EXPECT_FALSE(spec.pcm.samples.empty());
        EXPECT_TRUE(keys.insert(spec.key).second) << spec.key;
    }

    EXPECT_TRUE(keys.contains("flap"));
    EXPECT_TRUE(keys.contains("clear"));
    EXPECT_TRUE(keys.contains("die"));
}
