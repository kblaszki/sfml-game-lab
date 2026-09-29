#include <sgl/ui/TextLayout.hpp>

namespace sgl
{

float estimateWidth(std::string_view text, unsigned size, float advance)
{
    return static_cast<float>(text.size()) * static_cast<float>(size) * advance;
}

std::vector<std::string> wrap(std::string_view text, unsigned size, float maxWidth, float advance)
{
    if(text.empty())
    {
        return {};
    }

    std::vector<std::string> lines;
    std::string current;

    std::size_t i = 0;
    while(i <= text.size())
    {
        const std::size_t space = text.find(' ', i);
        const std::string_view word = text.substr(i, space == std::string_view::npos ? std::string_view::npos : space - i);

        if(current.empty())
        {
            current.assign(word);
        }
        else
        {
            std::string candidate = current;
            candidate.push_back(' ');
            candidate.append(word);
            if(estimateWidth(candidate, size, advance) <= maxWidth)
            {
                current = std::move(candidate);
            }
            else
            {
                lines.push_back(std::move(current));
                current.assign(word);
            }
        }

        if(space == std::string_view::npos)
        {
            break;
        }
        i = space + 1;
    }

    if(!current.empty() || lines.empty())
    {
        lines.push_back(std::move(current));
    }

    return lines;
}

} // namespace sgl
