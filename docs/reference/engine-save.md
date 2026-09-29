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
  - engine/save/include/sgl/save/Settings.hpp
  - engine/save/src/SaveError.cpp
  - engine/save/src/HighScoreTable.cpp
  - engine/save/src/SaveFormat.cpp
  - engine/save/src/SaveFile.cpp
  - engine/save/src/ScoreStore.cpp
  - engine/save/src/Settings.cpp
  - engine/save/CMakeLists.txt
  - tests/engine/save/HighScoreTableTest.cpp
  - tests/engine/save/SaveFormatTest.cpp
  - tests/engine/save/SaveFileTest.cpp
  - tests/engine/save/ScoreStoreTest.cpp
  - tests/engine/save/SettingsTest.cpp
related_docs:
  - source-layout.md
  - ../explanation/architecture.md
keywords: [eng, save, HighScoreTable, ScoreStore, Settings, SettingsStore, WindowMode, SaveError, parse, serialize, atomic, userDataDir]
last_reviewed: 2026-09-29
---

# Engine save

Static library target `sgl_save` (`namespace sgl`), linked `PUBLIC` to `sgl_core` only. No SFML. Include as `<sgl/save/X.hpp>`.

Persists high-score tables and window/volume settings as versioned text files under a platform user-data directory.

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

## Settings

`enum class WindowMode : uint8_t { Windowed, Fullscreen }`.

`kResolutions` is `{1280x720, 1600x900, 1920x1080}`.

```cpp
struct Settings {
    int masterVolume{80}; // 0..100
    WindowMode mode{WindowMode::Windowed};
    Vec2u resolution{1280, 720};
};
```

### Settings text format (`parseSettings` / `serializeSettings`)

Trailing newline. Example:

```
sgl-settings 1
volume=80
mode=windowed
resolution=1280x720
```

| Rule | Behavior |
|------|----------|
| Header | Must be exactly `sgl-settings 1`; missing/wrong → `SaveError::Corrupt` |
| `mode` | `windowed` or `fullscreen` |
| `volume` | Parsed int clamped to 0..100 |
| `resolution` | `WxH`; value not in `kResolutions` leaves the field at `{1280,720}` but parse still succeeds |
| Unknown keys | Ignored |

### SettingsStore

| API | Behavior |
|-----|----------|
| `SettingsStore::load(path)` | Missing or corrupt → default `Settings{}` (volume 80, windowed, 1280x720). Never throws |
| `SettingsStore::save(path, settings)` | `create_directories` on parent, then `writeTextFileAtomic` of `serializeSettings` |
