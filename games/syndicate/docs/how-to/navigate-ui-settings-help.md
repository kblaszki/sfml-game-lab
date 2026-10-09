---
title: Navigate UI, settings, and help
diataxis: how-to
audience: [ai, human]
related_code:
  - games/syndicate/app/include/syndicate/app/Bindings.hpp
  - games/syndicate/app/src/Bindings.cpp
  - games/syndicate/app/include/syndicate/app/ControlMap.hpp
  - games/syndicate/app/src/ControlMap.cpp
  - games/syndicate/app/include/syndicate/app/ActionBar.hpp
  - games/syndicate/app/include/syndicate/app/HelpScene.hpp
  - games/syndicate/app/include/syndicate/app/PauseMenuScene.hpp
  - games/syndicate/app/include/syndicate/app/PlayScene.hpp
  - games/syndicate/main.cpp
  - engine/save/include/sgl/save/Settings.hpp
  - engine/ui/include/sgl/ui/SettingsScene.hpp
  - engine/audio/include/sgl/audio/ScaledAudio.hpp
related_docs:
  - ../reference/app.md
  - ../reference/dice-animation.md
  - ../explanation/ui.md
  - ../../../../docs/reference/engine-ui.md
  - ../../../../docs/reference/engine-save.md
  - ../../../../docs/reference/pause-overlay.md
keywords: [syndicate, how-to, Bindings, ControlMap, HelpScene, Settings, ActionBar, PauseMenuScene]
last_reviewed: 2026-10-09
---

# Navigate UI, settings, and help

Player and developer guide for Syndicate controls, chrome, settings persistence, and help/pause scenes. Binding facts also live in [app.md](../reference/app.md).

## Keyboard and mouse (`bindSyndicate`)

`SyndicateActions` from `Bindings.cpp` / `Bindings.hpp`:

| Key | Action |
|-----|--------|
| **Space** | Roll (`actions.roll`) |
| **Enter** | End turn / menu confirm (`endTurn` and `menu.confirm`) |
| **C** | Auction bid (`bid`; `Key::B` does not exist in `sgl::Key`) |
| **P** | Pass bid |
| **Escape** | Pause (`RequestPause`) |
| **H** | Contextual help (`HelpScene`) |
| **Backspace** | Menu cancel |
| **Up** / **Down** | Menu focus |
| **Left** / **Right** | Settings nudges / help tabs |
| **Num1**–**Num9** | Action-bar slots |

Mouse: click glowing path tiles in `AwaitPath`; click board tiles in Manage / purchase to filter the bar; click action-bar buttons; hover for tooltips. While dice/pawn presentation is busy, click or Space **fast-forwards** animation instead of issuing a command ([dice-animation.md](../reference/dice-animation.md)).

`controlMap()` / `hintLine(phase)` in `ControlMap.cpp` mirror these rows for help and the on-screen hint (also drawn above the ActionBar). On the first `PlayScene` update, two short `HudModel` notes show the current phase hint and remind you to press **H** for Help.

## ActionBar and tooltips

`ActionBar` rebuilds from `Match::legalCommands`, drops `ChooseDestination`, caps at nine slots preferring `selectedTile`, and lays out in `kActionBar`. In `Manage`, while fewer than three trades have been proposed, PlayScene passes `offerTrade` so a **Trade** slot is appended (it counts toward the nine). Num1–Num9 and pointer release activate a slot. The Trade slot opens `TradeComposeOverlay` instead of applying a `Command`. Other slots apply the command. Hotspots feed `TooltipSystem` (`title` = label, `body` = tooltip from `describeCommand`). Enter is **not** a bar click (legacy EndTurn stays on Enter).

PlayScene also hotspots board tiles (`describeTile`), hand cards (`describeCard`), market chart rows, and player panels.

## Settings

`sgl::SettingsScene` (engine) edits:

| Row | Field | Nudge |
|-----|-------|-------|
| Master volume | `masterVolume` 0..100 | ±10 |
| Window | `WindowMode::Windowed` / `Fullscreen` | toggle |
| Resolution | `kResolutions`: 1280×720, 1600×900, 1920×1080 | cycle |
| Back | Pop scene | confirm / cancel |

`main.cpp` loads `userDataDir("syndicate") / "settings.txt"` via `SettingsStore::load` (header `sgl-settings 1`; missing → defaults). Applies `SfmlPlatform::setDisplay(mode, resolution)` and wraps audio in `ScaledAudio` with `setMaster(volume/100)`. `onSettingsChanged` updates services, display, master volume, and `SettingsStore::save` (save errors → `cerr`, no quit).

Main menu and pause both push `SettingsScene` with `services.settings` / `onSettingsChanged`.

### Add a settings entry

1. Extend `sgl::Settings` in `engine/save/include/sgl/save/Settings.hpp` and update `parseSettings` / `serializeSettings` / tests.
2. Add a `SettingsScene::Row`, `rowLabel`, and `nudge` branch in `engine/ui` `SettingsScene`.
3. Apply the field in Syndicate’s `onSettingsChanged` (and startup) in `games/syndicate/main.cpp`.
4. Document in [engine-save.md](../../../../docs/reference/engine-save.md) / [engine-ui.md](../../../../docs/reference/engine-ui.md).

## Help scene

`HelpScene(phaseIndex, MenuActions, FontId)` builds tabs from unique `controlMap()` phase names (Roll, Path, Purchase, Auction, Trade, Manage, Global). Starts on `phaseIndex` clamped to tab count. Left/Right or tab clicks change tabs; cancel/Back → `PopScene`. PlayScene **H** selects the tab matching the current `Phase`. Main menu / pause open tab 0.

### Add a help page

1. Add `ControlRow` entries in `ControlMap.cpp` with a new or existing `.phase` string (new phase name → new tab).
2. Ensure `hintLine` covers any new `Phase` alternative if Match gains one.
3. Extend `tests/syndicate/app/ControlMapTest.cpp` / `HelpSceneTest.cpp`.

## Pause menu

`PauseMenuScene`: Resume or cancel → `PopScene`; Settings → `SettingsScene`; Help → `HelpScene` tab 0; Quit → `QuitApp`. Traits: `opaque` false, `blocksUpdate` true (not `sgl::PauseOverlay`). See [pause-overlay.md](../../../../docs/reference/pause-overlay.md).
