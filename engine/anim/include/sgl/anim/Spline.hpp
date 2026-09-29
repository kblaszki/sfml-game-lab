#pragma once

#include <sgl/core/Vec2.hpp>
#include <span>
#include <vector>

namespace sgl
{

[[nodiscard]] Vec2f cubicBezier(Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3, float t);

[[nodiscard]] std::vector<Vec2f> catmullRom(std::span<const Vec2f> points, int samplesPerSegment);

class ArcLengthTable
{
public:
    void build(std::span<const Vec2f> points);

    [[nodiscard]] float length() const noexcept
    {
        return length_;
    }

    [[nodiscard]] Vec2f at(float distance) const;

private:
    std::vector<Vec2f> points_;
    std::vector<float> cumulative_;
    float length_{};
};

} // namespace sgl
