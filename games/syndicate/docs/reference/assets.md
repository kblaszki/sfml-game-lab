---
title: Syndicate assets
diataxis: reference
audience: [ai, human]
related_code:
  - engine/resources/include/sgl/resources/Atlas.hpp
  - engine/resources/src/Atlas.cpp
  - games/syndicate/app/include/syndicate/app/SpriteBank.hpp
  - games/syndicate/app/src/SpriteBank.cpp
  - games/syndicate/app/src/Theme.cpp
  - games/syndicate/sim/include/syndicate/sim/Cards.hpp
  - games/syndicate/main.cpp
  - assets/fonts/
related_docs:
  - app.md
  - ../../../../docs/reference/engine-resources.md
  - ../../../../docs/reference/engine-sfml.md
  - ../../../../docs/reference/source-layout.md
  - ../explanation/ui.md
keywords: [syndicate, atlas, SpriteBank, PNG, assets, theme, procedural fallback]
last_reviewed: 2026-10-09
---

# Syndicate assets

Optional raster pack for Syndicate presentation. A minimal procedural `assets/syndicate/atlas.png` + `atlas.txt` ships with sector/tile/card color chips so `SpriteBank` can load in `main.cpp`; missing region names still fall back to procedural chrome (`MeshCmd` / shapes). Design size **1280×720**.

## Format rules

- **PNG-32** sRGB with **straight alpha** (not premultiplied).
- Atlas max **2048×2048**, **2px** padding between regions, **1px** extrude.
- Manifest line format: `name x y w h` (`#` comments and blank lines allowed). Names may contain slashes (`sector/ai`).
- Runtime files (optional): `assets/syndicate/atlas.png` + `assets/syndicate/atlas.txt`.
- `SpriteBank::sprite` returns `nullopt` when the name is absent → caller draws procedural fallback (no sprite).

## Asset table

| Category | px @1x | @2x | Format / alpha | Count | Notes |
|----------|--------|-----|----------------|-------|-------|
| Sector icons | 64 | 128 | PNG-32 straight | 6 | `ai`, `clean-energy`, `biotech`, `cybersec`, `logistics`, `fintech` (the `Sector` enum). White glyph, tinted by sector color. Atlas names `sector/<name>`. |
| Special tiles | 64 | 128 | PNG-32 straight | 5 | `tile/payday`, `tile/exchange`, `tile/hub`, `tile/audit`, `tile/grant`. |
| Upgrade badges | 24 | 48 | PNG-32 straight | 3 | `level/1` … `level/3`. Level 0 has no badge. |
| Pawns | 48 (+ 48×24 shadow) | 96 (+ 96×48) | PNG-32 straight | 1+optional 4 | White base tinted to 4 players; optional silhouette variants. Origin bottom center. |
| Card frame | 160×224 | 320×448 | PNG-32 straight | 2 | Front and back; 12px rounded corners. |
| Card art | 128×96 | 256×192 | PNG-32 straight | 8 | Matches `CardKind`: HostileTakeover, Audit, MarketCrash, ShortSqueeze, Lobbying, Embargo, Subsidy, Insider. |
| Buttons | 48×48 (9-slice, border 12) | 96 | PNG-32 straight | 4 states | `idle` / `hover` / `pressed` / `disabled`. Procedural fallback: rounded rect r=8. |
| Panel / tooltip | 64×64 (9-slice, border 16) | 128 | PNG-32 straight | 2 | Shared chrome slices. |
| UI icons | 32×32 | 64 | PNG-32 straight | ~16 | `cash`, `share`, `dividend`, `fee`, `auction`, `trade`, `upgrade`, `sell`, `card`, `dice`, `settings`, `help`, `volume`, `fullscreen`, `arrow`, `lock`. |
| Dice faces | 96×96 | 192 | PNG-32 straight | 6 (+ optional strip) | Optional tumble strip 1152×96 (12 frames). Procedural fallback: rounded rect + pips. |
| Glow | 128×128 | 256 | PNG-32 straight | 1 | Radial white → transparent. Optional; mesh glow is the fallback. |
| Backgrounds | 1920×1080 | — | Opaque PNG or JPG | 2 | `bg/menu`, `bg/board`. Fallback procedural gradient. |
| Fonts | — | — | TTF | 2 | Display `fonts/upheavtt.ttf`, body `fonts/VCR_OSD_MONO_1.001.ttf` (already under `assets/fonts/`). |

## Theme colors (`defaultTheme`)

From `Theme.cpp` (RGB → hex):

| Role | Hex |
|------|-----|
| Sector AI | `#50B4FF` |
| Sector CleanEnergy | `#5AD278` |
| Sector Biotech | `#C878E6` |
| Sector CyberSec | `#FF6E5A` |
| Sector Logistics | `#FFBE46` |
| Sector Fintech | `#64DCC8` |
| Edge | `#465064` |
| Payday | `#F0D250` |
| Exchange | `#B4A0FF` |
| Hub | `#C8C8D2` |
| Audit | `#FF648C` |
| Grant | `#78E6B4` |
| Highlight | `#FFFF78` |
| Pawn 0–3 | `#FF5A5A`, `#5AA0FF`, `#FFC83C`, `#B4FF78` |

Tint each `sector/<name>` glyph with `Theme::sector` for that `Sector`.

## Layout sketch (1280×720)

```
+------------------------------------------------------------------+
| left panel x 16..340                                             |
|                                                  right chart     |
|                                                  y 16..300       |
|                    board origin {640,340}                        |
|                    radius 270                                    |
|                                                  right log       |
|                                                  y 316..636      |
| action bar y 652..708                                            |
+------------------------------------------------------------------+
```

## Runtime load path

`main.cpp` loads the atlas only when **both** `atlas.png` and `atlas.txt` exist under `ASSET_DIR/syndicate/`: read text → `parseAtlas` → `SfmlAssets::textureFile(..., smooth=true)` → `SpriteBank{id, manifest}`. Any failure keeps an empty `SpriteBank` (game continues).

## SVG → PNG export note

Authoring SVGs would live in `assets-src/` (not committed). Export produces `assets/syndicate/atlas.png` plus matching `atlas.txt` with `name x y w h` rows. The shipped tree may omit both files entirely.
