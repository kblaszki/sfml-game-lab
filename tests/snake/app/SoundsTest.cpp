#include <gtest/gtest.h>
#include <set>
#include <snake/app/Sounds.hpp>
#include <string_view>

TEST(SoundsTest, eatAndDieAreNonEmptyAndUnique)
{
    const auto specs = sgl::snake::soundSpecs();
    ASSERT_EQ(specs.size(), 2u);

    std::set<std::string_view> keys;
    for(const sgl::snake::SoundSpec& spec: specs)
    {
        EXPECT_FALSE(spec.key.empty());
        EXPECT_GT(spec.pcm.sampleRate, 0u);
        EXPECT_FALSE(spec.pcm.samples.empty());
        EXPECT_TRUE(keys.insert(spec.key).second) << spec.key;
    }

    EXPECT_TRUE(keys.contains("eat"));
    EXPECT_TRUE(keys.contains("die"));
}
