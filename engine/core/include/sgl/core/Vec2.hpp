#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <optional>

namespace sgl
{

template<std::floating_point T>
struct Vec2
{
    T x{};
    T y{};

    constexpr Vec2& operator+=(const Vec2& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    friend constexpr Vec2 operator+(Vec2 a, const Vec2& b) noexcept
    {
        a += b;
        return a;
    }

    friend constexpr Vec2 operator-(Vec2 a, const Vec2& b) noexcept
    {
        a.x -= b.x;
        a.y -= b.y;
        return a;
    }

    friend constexpr Vec2 operator*(Vec2 v, T scalar) noexcept
    {
        v.x *= scalar;
        v.y *= scalar;
        return v;
    }

    friend constexpr Vec2 operator*(T scalar, Vec2 v) noexcept
    {
        return v * scalar;
    }

    constexpr auto operator<=>(const Vec2&) const = default;
    constexpr bool operator==(const Vec2&) const = default;
};

using Vec2f = Vec2<float>;

struct Vec2u
{
    std::uint32_t x{};
    std::uint32_t y{};

    constexpr bool operator==(const Vec2u&) const = default;
    constexpr auto operator<=>(const Vec2u&) const = default;
};

template<std::floating_point T>
constexpr T dot(const Vec2<T>& a, const Vec2<T>& b) noexcept
{
    return a.x * b.x + a.y * b.y;
}

template<std::floating_point T>
constexpr T lengthSq(const Vec2<T>& v) noexcept
{
    return dot(v, v);
}

template<std::floating_point T>
constexpr std::optional<Vec2<T>> normalized(const Vec2<T>& v) noexcept
{
    const T lenSq = lengthSq(v);
    if(lenSq == T{})
    {
        return std::nullopt;
    }
    const T invLen = T{1} / std::sqrt(lenSq);
    return Vec2<T>{v.x * invLen, v.y * invLen};
}

} // namespace sgl
