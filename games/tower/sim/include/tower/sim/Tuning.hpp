#pragma once

namespace sgl::tower
{

struct Tuning
{
    float moveAccel{2400.f};
    float maxRunSpeed{320.f};
    float iceFriction{1.8f};
    float gravity{1800.f};
    float jumpSpeed{780.f};
    float playerW{40.f};
    float playerH{56.f};
    float platformH{20.f};
    float startPlatformW{280.f};
    float minPlatformW{80.f};
    float deathMargin{40.f};
    float bandHeight{120.f};
    float wallBounce{0.95f};
    float wallKick{220.f};
    float heightAccel{0.00015f};
    float heightAccelCap{0.55f};
    float bonusSize{28.f};
    float bonusChance{0.22f};
    float speedBoostMul{1.45f};
    float jumpBoostMul{1.35f};
    float boostDuration{4.f};
    int pointsBonus{50};
};

} // namespace sgl::tower
