#include <cstddef>
#include <syndicate/app/MarketHistory.hpp>
#include <utility>

namespace sgl::syndicate
{

void MarketHistory::record(const Market& market)
{
    if(count_ < kCapacity)
    {
        for(std::size_t i = 0; i < market.index.size(); ++i)
        {
            samples_[i][count_] = market.index[i];
        }
        ++count_;
        return;
    }

    for(std::size_t i = 0; i < samples_.size(); ++i)
    {
        for(std::size_t j = 1; j < kCapacity; ++j)
        {
            samples_[i][j - 1] = samples_[i][j];
        }
        samples_[i][kCapacity - 1] = market.index[i];
    }
}

std::span<const Percent> MarketHistory::series(Sector sector) const
{
    const auto idx = static_cast<std::size_t>(std::to_underlying(sector));
    return std::span<const Percent>{samples_[idx].data(), count_};
}

} // namespace sgl::syndicate
