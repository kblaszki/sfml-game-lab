#include <sgl/anim/Spline.hpp>

#include <algorithm>
#include <cmath>

namespace sgl
{

namespace
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

[[nodiscard]] Vec2f sampleCatmullRom(Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3, float t) noexcept
{
    const float t2 = t * t;
    const float t3 = t2 * t;
    return (p1 * 2.f + (p2 - p0) * t + (p0 * 2.f - p1 * 5.f + p2 * 4.f - p3) * t2 +
            (p1 * 3.f - p0 - p2 * 3.f + p3) * t3) *
           0.5f;
}

} // namespace

Vec2f cubicBezier(Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3, float t)
{
    t = clamp01(t);
    const float u = 1.f - t;
    const float uu = u * u;
    const float tt = t * t;
    return p0 * (uu * u) + p1 * (3.f * uu * t) + p2 * (3.f * u * tt) + p3 * (tt * t);
}

std::vector<Vec2f> catmullRom(std::span<const Vec2f> points, int samplesPerSegment)
{
    if(points.size() < 2 || samplesPerSegment < 1)
    {
        return {};
    }

    std::vector<Vec2f> out;
    out.reserve(1u + (points.size() - 1u) * static_cast<std::size_t>(samplesPerSegment));
    out.push_back(points.front());

    const std::size_t last = points.size() - 1u;
    for(std::size_t i = 0; i < last; ++i)
    {
        const Vec2f p0 = i == 0 ? points[0] : points[i - 1];
        const Vec2f p1 = points[i];
        const Vec2f p2 = points[i + 1];
        const Vec2f p3 = (i + 2 <= last) ? points[i + 2] : points[last];

        for(int s = 1; s <= samplesPerSegment; ++s)
        {
            const float t = static_cast<float>(s) / static_cast<float>(samplesPerSegment);
            out.push_back(sampleCatmullRom(p0, p1, p2, p3, t));
        }
    }

    return out;
}

void ArcLengthTable::build(std::span<const Vec2f> points)
{
    points_.clear();
    cumulative_.clear();
    length_ = 0.f;

    if(points.size() < 2)
    {
        return;
    }

    points_.assign(points.begin(), points.end());
    cumulative_.resize(points_.size());
    cumulative_[0] = 0.f;

    for(std::size_t i = 1; i < points_.size(); ++i)
    {
        const Vec2f d = points_[i] - points_[i - 1];
        length_ += std::sqrt(lengthSq(d));
        cumulative_[i] = length_;
    }
}

Vec2f ArcLengthTable::at(float distance) const
{
    if(points_.empty())
    {
        return {};
    }
    if(distance <= 0.f)
    {
        return points_.front();
    }
    if(distance >= length_)
    {
        return points_.back();
    }

    const auto it = std::lower_bound(cumulative_.begin(), cumulative_.end(), distance);
    const std::size_t i = static_cast<std::size_t>(it - cumulative_.begin());
    const float segStart = cumulative_[i - 1];
    const float segLen = cumulative_[i] - segStart;
    const float u = segLen > 0.f ? (distance - segStart) / segLen : 0.f;
    return points_[i - 1] + (points_[i] - points_[i - 1]) * u;
}

} // namespace sgl
