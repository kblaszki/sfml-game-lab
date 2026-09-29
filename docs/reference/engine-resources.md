---
title: Engine resource cache
diataxis: reference
audience: [ai, human]
related_code:
  - engine/resources/include/sgl/resources/ResourceCache.hpp
  - engine/resources/include/sgl/resources/ResourceError.hpp
  - engine/resources/include/sgl/resources/Atlas.hpp
  - engine/resources/src/Atlas.cpp
  - engine/resources/CMakeLists.txt
  - engine/render/include/sgl/render/AssetIds.hpp
  - tests/engine/resources/ResourceCacheTest.cpp
  - tests/engine/resources/AtlasTest.cpp
related_docs:
  - source-layout.md
  - engine-render.md
  - syndicate-assets.md
  - ../explanation/architecture.md
keywords: [eng, ResourceCache, ResourceError, Atlas, AtlasManifest, parseAtlas, TextureId, FontId, load, get, find]
last_reviewed: 2026-09-29
---

# Engine resource cache

Keyed cache and atlas manifest in `namespace sgl`, included as `<sgl/resources/X.hpp>`. Target `sgl_resources` is a **STATIC** library that PUBLIC-links `sgl_core` (`Atlas.cpp` + headers). `ResourceCache.hpp` remains header-only and is included the same way. No SFML dependency.

Asset identity types `TextureId` / `FontId` come from `<sgl/render/AssetIds.hpp>`; this module does not redefine them. Typical usage is `ResourceCache<TextureId, T>` / `ResourceCache<FontId, T>` with a caller-supplied loader.

## ResourceError

`enum class ResourceError { NotFound, DecodeFailed, Unsupported }`.

`describe(error)` returns a stable `std::string_view` label (`"NotFound"`, `"DecodeFailed"`, `"Unsupported"`).

## ResourceCache\<Id, T\>

Storage is a `std::vector<std::unique_ptr<T>>` plus a parallel `std::vector<std::string>` of keys. `Id::id` is `index + 1`; `0` is invalid.

| API | Behavior |
|-----|----------|
| `load(key, loader)` | If `key` exists, returns the existing `Id` and does not call `loader`. Otherwise invokes `loader`; on success stores `T` and returns a new `Id`; on error returns `ResourceError` and leaves `size()` unchanged |
| `get(id)` | Pointer to stored `T`, or `nullptr` for invalid / out-of-range `id`. Pointers remain valid across later successful inserts |
| `find(key)` | `std::optional<Id>` for an existing key |
| `size()` | Number of stored resources |

`Loader` must satisfy `ResourceLoader`: invocable as `Loader&`, with `std::invoke_result_t<Loader&>` equal to `std::expected<T, ResourceError>`.

## Atlas

`AtlasRect` holds integer `x y w h`. `AtlasEntry` pairs a `name` with a rect. `AtlasManifest` stores a `std::vector<AtlasEntry>` and offers `find(name)` / `size()`.

`parseAtlas(text)` reads lines `name x y w h` (`#` comments and blank lines allowed; names may contain slashes). Rejects missing fields, non-positive `w`/`h`, and duplicate names (`std::expected` error string mentions the problem). Builds the manifest via its vector constructor.
