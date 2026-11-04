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
};

} // namespace sgl::tower
