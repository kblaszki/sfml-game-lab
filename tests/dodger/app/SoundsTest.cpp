#include <dodger/app/Sounds.hpp>
#include <gtest/gtest.h>
#include <set>
#include <string_view>

TEST(SoundsTest, dieIsNonEmpty)
{
    const auto specs = sgl::dodger::soundSpecs();
    ASSERT_EQ(specs.size(), 1u);

    std::set<std::string_view> keys;
    for(const sgl::dodger::SoundSpec& spec: specs)
    {
        EXPECT_FALSE(spec.key.empty());
        EXPECT_GT(spec.pcm.sampleRate, 0u);
        EXPECT_FALSE(spec.pcm.samples.empty());
        EXPECT_TRUE(keys.insert(spec.key).second) << spec.key;
    }

    EXPECT_TRUE(keys.contains("die"));
}
