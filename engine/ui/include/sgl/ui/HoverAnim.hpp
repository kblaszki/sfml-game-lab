#pragma once

#include <cstddef>
#include <optional>
#include <sgl/core/Time.hpp>
#include <vector>

namespace sgl
{

class HoverAnim
{
public:
    void update(std::optional<std::size_t> hovered, std::size_t count, Seconds dt);
    [[nodiscard]] float scale(std::size_t index) const;

private:
    std::vector<float> scales_;
};

} // namespace sgl
