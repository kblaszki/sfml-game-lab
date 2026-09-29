#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Money.hpp>

namespace sgl::syndicate
{

class MarketHistory
{
public:
    static constexpr std::size_t kCapacity = 32;

    void record(const Market& market);
    [[nodiscard]] std::span<const Percent> series(Sector sector) const;

private:
    std::array<std::array<Percent, kCapacity>, 6> samples_{};
    std::size_t count_{};
};

} // namespace sgl::syndicate
