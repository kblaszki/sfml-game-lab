#include <gtest/gtest.h>
#include <set>
#include <string_view>
#include <tower/app/Sounds.hpp>

TEST(SoundsTest, jumpLandAndDieAreNonEmptyAndUnique)
{
    const auto specs = sgl::tower::soundSpecs();
    ASSERT_EQ(specs.size(), 3u);

    std::set<std::string_view> keys;
    for(const sgl::tower::SoundSpec& spec: specs)
    {
        EXPECT_FALSE(spec.key.empty());
        EXPECT_GT(spec.pcm.sampleRate, 0u);
        EXPECT_FALSE(spec.pcm.samples.empty());
        EXPECT_TRUE(keys.insert(spec.key).second) << spec.key;
    }

    EXPECT_TRUE(keys.contains("jump"));
    EXPECT_TRUE(keys.contains("land"));
    EXPECT_TRUE(keys.contains("die"));
}
