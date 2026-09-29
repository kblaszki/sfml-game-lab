#pragma once

#include <cstdint>

namespace sgl::syndicate
{

struct Money
{
    std::int64_t v{};

    auto operator<=>(const Money&) const = default;
};

[[nodiscard]] constexpr Money operator+(Money a, Money b) noexcept
{
    return Money{a.v + b.v};
}

[[nodiscard]] constexpr Money operator-(Money a, Money b) noexcept
{
    return Money{a.v - b.v};
}

[[nodiscard]] constexpr Money operator*(Money m, int n) noexcept
{
    return Money{m.v * n};
}

[[nodiscard]] constexpr Money operator*(int n, Money m) noexcept
{
    return Money{m.v * n};
}

struct Percent
{
    std::int32_t v{100};
};

[[nodiscard]] constexpr Money scale(Money m, Percent p) noexcept
{
    return Money{m.v * p.v / 100};
}

} // namespace sgl::syndicate
