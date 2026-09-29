#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace sgl
{

[[nodiscard]] float estimateWidth(std::string_view text, unsigned size, float advance = 0.6f);

[[nodiscard]] std::vector<std::string> wrap(std::string_view text, unsigned size, float maxWidth, float advance = 0.6f);

} // namespace sgl
