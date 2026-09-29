#pragma once

#include <cstdint>
#include <sgl/core/Random.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/Theme.hpp>

namespace sgl::syndicate
{

/// Cosmetic d4+d4 tumble. Uses a dedicated Pcg32; never touches Match RNG.
class DiceAnimator
{
public:
    explicit DiceAnimator(std::uint64_t seed);

    void start(int finalRoll);
    void update(sgl::Seconds dt);
    [[nodiscard]] bool busy() const noexcept;
    void fastForward();
    void render(sgl::RenderQueue& queue, sgl::Vec2f center, const Theme& theme) const;
    [[nodiscard]] int faceA() const noexcept;
    [[nodiscard]] int faceB() const noexcept;

private:
    void splitFinal(int finalRoll);
    void randomizeFaces();

    sgl::Pcg32 rng_;
    bool active_{false};
    bool popping_{false};
    float elapsed_{0.f};
    float nextChangeAt_{0.f};
    int intervalK_{0};
    int faceA_{1};
    int faceB_{1};
    int finalA_{1};
    int finalB_{1};
    float scale_{1.f};
};

} // namespace sgl::syndicate
