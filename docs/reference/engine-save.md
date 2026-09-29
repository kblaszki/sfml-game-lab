---
title: Engine save
diataxis: reference
audience: [ai, human]
related_code:
  - engine/save/include/sgl/save/SaveError.hpp
  - engine/save/include/sgl/save/HighScoreTable.hpp
  - engine/save/include/sgl/save/SaveFormat.hpp
  - engine/save/include/sgl/save/SaveFile.hpp
  - engine/save/include/sgl/save/ScoreStore.hpp
  - engine/save/src/SaveError.cpp
  - engine/save/src/HighScoreTable.cpp
  - engine/save/src/SaveFormat.cpp
  - engine/save/src/SaveFile.cpp
  - engine/save/src/ScoreStore.cpp
  - engine/save/CMakeLists.txt
  - tests/engine/save/HighScoreTableTest.cpp
  - tests/engine/save/SaveFormatTest.cpp
  - tests/engine/save/SaveFileTest.cpp
  - tests/engine/save/ScoreStoreTest.cpp
related_docs:
  - source-layout.md
  - ../explanation/architecture.md
keywords: [eng, save, HighScoreTable, ScoreStore, SaveError, parse, serialize, atomic, userDataDir]
last_reviewed: 2026-09-29
---

# Engine save

Static library target `sgl_save` (`namespace sgl`), linked `PUBLIC` to `sgl_core` only. No SFML. Include as `<sgl/save/X.hpp>`.

Persists high-score tables as versioned text files under a platform user-data directory.

## SaveError

`enum class SaveError { NotFound, Io, Corrupt }`.

`describe(error)` returns a stable C string label (`"NotFound"`, `"Io"`, `"Corrupt"`).

## HighScoreTable

Fixed-capacity descending score list. Constructor takes `capacity`. Ties keep the older (earlier-inserted) score above a new equal score.

| API | Behavior |
|-----|----------|
| `qualifies(score)` | `true` when the table is not full, or when `score` beats the current lowest entry |
| `insert(score)` | Inserts in descending order; returns 0-based rank, or `nullopt` if dropped |
| `entries()` | `std::span<const uint32_t>` over current scores (highest first) |

## Text format (`parse` / `serialize`)

First line must be exactly `sgl-highscores 1`, then one unsigned integer per line. Wrong header, non-numeric tokens, or negatives yield `SaveError::Corrupt`. `parse(text, capacity)` builds a `HighScoreTable`; `serialize` writes the header and current entries.

## Files

| API | Behavior |
|-----|----------|
| `readTextFile(path)` | Whole-file read; missing path → `NotFound`; other failures → `Io` |
| `writeTextFileAtomic(path, contents)` | Write `path` + `.tmp`, then `std::filesystem::rename`. On rename failure (e.g. MinGW rename-over-existing), remove the target, retry rename once; still remove the `.tmp` if the retry fails |
| `userDataDir(app)` | Windows: `APPDATA/<app>`; else `XDG_DATA_HOME/<app>` or `HOME/.local/share/<app>`; missing env → `Io` |

## ScoreStore

Convenience load/save over the text format and atomic write. Never throws.

| API | Behavior |
|-----|----------|
| `ScoreStore::load(path, capacity)` | `readTextFile` + `parse`. Missing, corrupt, or other IO failure → empty `HighScoreTable{capacity}` |
| `ScoreStore::save(path, table)` | `create_directories` on the parent path, then `writeTextFileAtomic(path, serialize(table))`. Returns the write `SaveError` if any |
