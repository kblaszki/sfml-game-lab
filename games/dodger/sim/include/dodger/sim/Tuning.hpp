#pragma once

namespace sgl::dodger
{

struct Tuning
{
    float paddleSpeed{480.f};
    float paddleW{64.f};
    float paddleH{24.f};
    float hazardW{40.f};
    float hazardH{40.f};
    float hazardSpeed{300.f};
    float spawnInterval{0.5f};
};

} // namespace sgl::dodger
