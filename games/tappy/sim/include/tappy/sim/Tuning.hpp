#pragma once

namespace sgl::tappy
{

struct Tuning
{
    float gravity{2200.f};
    float flapSpeed{520.f};
    float maxFall{900.f};
    float maxRise{700.f};
    float playerW{48.f};
    float playerH{36.f};
    float scrollBase{220.f};
    float scrollAccel{0.015f};
    float scrollCap{420.f};
    float gateWidth{80.f};
    float gateSpacing{320.f};
    float gapStart{200.f};
    float gapMin{110.f};
    float gapShrink{0.02f};
    float gapCenterMargin{80.f};
    float playerSpawnX{280.f};
    float playerSpawnY{360.f};
};

} // namespace sgl::tappy
