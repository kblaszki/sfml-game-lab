#include <sgl/save/SaveFile.hpp>
#include <sgl/save/SaveFormat.hpp>
#include <sgl/save/ScoreStore.hpp>
#include <system_error>

namespace sgl
{

HighScoreTable ScoreStore::load(const std::filesystem::path& path, std::size_t capacity)
{
    const auto text = readTextFile(path);
    if(!text)
    {
        return HighScoreTable{capacity};
    }

    const auto table = parse(*text, capacity);
    if(!table)
    {
        return HighScoreTable{capacity};
    }
    return *table;
}

std::expected<void, SaveError> ScoreStore::save(const std::filesystem::path& path, const HighScoreTable& table)
{
    const auto parent = path.parent_path();
    if(!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if(ec)
        {
            return std::unexpected(SaveError::Io);
        }
    }
    return writeTextFileAtomic(path, serialize(table));
}

} // namespace sgl
