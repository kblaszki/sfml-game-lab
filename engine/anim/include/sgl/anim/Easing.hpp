#pragma once

namespace sgl
{

using EaseFn = float (*)(float);

namespace detail
{

[[nodiscard]] constexpr float clamp01(float t) noexcept
{
    if(t < 0.f)
    {
        return 0.f;
    }
    if(t > 1.f)
    {
        return 1.f;
    }
    return t;
}

} // namespace detail

[[nodiscard]] constexpr float linear(float t) noexcept
{
    return detail::clamp01(t);
}

[[nodiscard]] constexpr float easeInQuad(float t) noexcept
{
    t = detail::clamp01(t);
    return t * t;
}

[[nodiscard]] constexpr float easeOutCubic(float t) noexcept
{
    t = detail::clamp01(t);
    const float u = 1.f - t;
    return 1.f - u * u * u;
}

[[nodiscard]] constexpr float easeInOutCubic(float t) noexcept
{
    t = detail::clamp01(t);
    if(t < 0.5f)
    {
        return 4.f * t * t * t;
    }
    const float u = -2.f * t + 2.f;
    return 1.f - (u * u * u) / 2.f;
}

[[nodiscard]] constexpr float easeOutBack(float t) noexcept
{
    t = detail::clamp01(t);
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.f;
    const float u = t - 1.f;
    return 1.f + c3 * (u * u * u) + c1 * (u * u);
}

} // namespace sgl
