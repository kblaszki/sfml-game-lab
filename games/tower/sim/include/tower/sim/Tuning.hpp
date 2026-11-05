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
    float wallThickness{120.f};
    float wallKickBase{380.f};
    float wallKickSpeedMul{0.85f};
    float wallComboMul{0.08f};
    float heightAccel{0.00015f};
    float heightAccelCap{0.55f};
    float bonusSize{28.f};
    float bonusChance{0.22f};
    float speedBoostMul{1.45f};
    float jumpBoostMul{1.35f};
    float boostDuration{4.f};
    int pointsBonus{50};
    float edgeJumpMargin{18.f};
    float edgeJumpMul{1.18f};
    float comboWindow{0.9f};
    float comboJumpScale{0.04f};
    int comboCap{25};
    int comboPoints{10};
    float maxSiblingGap{420.f};
};

} // namespace sgl::tower
