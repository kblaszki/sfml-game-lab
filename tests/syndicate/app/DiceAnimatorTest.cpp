#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <syndicate/app/DiceAnimator.hpp>

using sgl::syndicate::DiceAnimator;

TEST(DiceAnimatorTest, tumbleIntervalsGrowAndSettleOnSeven)
{
    DiceAnimator dice{42};
    dice.start(7);
    EXPECT_TRUE(dice.busy());

    int faceChanges = 0;
    int prevA = dice.faceA();
    int prevB = dice.faceB();
    float lastChangeTime = 0.f;
    float lastInterval = 0.f;
    float elapsed = 0.f;
    const float step = 1.f / 60.f;
    bool haveInterval = false;

    while(elapsed < 1.2f + step * 0.5f)
    {
        dice.update(sgl::Seconds{step});
        elapsed += step;
        if(dice.faceA() != prevA || dice.faceB() != prevB)
        {
            // Interval growth applies to tumble steps before the final snap at 0.9s.
            if(elapsed < 0.9f)
            {
                const float interval = elapsed - lastChangeTime;
                if(haveInterval)
                {
                    EXPECT_GE(interval + 1e-4f, lastInterval);
                }
                lastInterval = interval;
                haveInterval = true;
                lastChangeTime = elapsed;
                ++faceChanges;
            }
            prevA = dice.faceA();
            prevB = dice.faceB();
        }
    }

    EXPECT_GE(faceChanges, 6);
    EXPECT_EQ(dice.faceA() + dice.faceB(), 7);
    EXPECT_GE(dice.faceA(), 1);
    EXPECT_LE(dice.faceA(), 4);
    EXPECT_GE(dice.faceB(), 1);
    EXPECT_LE(dice.faceB(), 4);
    EXPECT_FALSE(dice.busy());
}

TEST(DiceAnimatorTest, fastForwardSnapsFacesAndClearsBusy)
{
    DiceAnimator dice{99};
    dice.start(7);
    EXPECT_TRUE(dice.busy());
    dice.fastForward();
    EXPECT_FALSE(dice.busy());
    EXPECT_EQ(dice.faceA() + dice.faceB(), 7);
    EXPECT_GE(dice.faceA(), 1);
    EXPECT_LE(dice.faceA(), 4);
    EXPECT_GE(dice.faceB(), 1);
    EXPECT_LE(dice.faceB(), 4);
}
