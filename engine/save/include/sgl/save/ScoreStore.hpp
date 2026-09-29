#pragma once

#include <expected>
#include <filesystem>
#include <sgl/save/HighScoreTable.hpp>
#include <sgl/save/SaveError.hpp>

namespace sgl
{

struct ScoreStore
{
    // NotFound or Corrupt -> empty table of the given capacity. Other IO errors -> empty table too (do not throw).
    [[nodiscard]] static HighScoreTable load(const std::filesystem::path& path, std::size_t capacity);

    // create_directories on parent, then writeTextFileAtomic. Returns the write error.
    [[nodiscard]] static std::expected<void, SaveError> save(const std::filesystem::path& path,
                                                             const HighScoreTable& table);
};

} // namespace sgl
