#pragma once

namespace sgl::asteroids
{

struct Tuning
{
    float thrustAccel{320.f};
    float rotateSpeed{3.5f}; // rad/s
    float bulletSpeed{520.f};
    float bulletLifetime{1.2f};
    float fireCooldown{0.22f};
    float maxShipSpeed{380.f};
    float shipRadius{12.f};
    float largeRadius{48.f};
    float mediumRadius{28.f};
    float smallRadius{14.f};
    int largeScore{100};
    int mediumScore{50};
    int smallScore{25};
    int initialLargeRocks{4};
};

} // namespace sgl::asteroids
