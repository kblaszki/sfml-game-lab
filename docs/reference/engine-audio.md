---
title: Engine audio
diataxis: reference
audience: [ai, human]
related_code:
  - engine/audio/include/sgl/audio/SoundId.hpp
  - engine/audio/include/sgl/audio/Pcm.hpp
  - engine/audio/include/sgl/audio/Waveform.hpp
  - engine/audio/include/sgl/audio/Envelope.hpp
  - engine/audio/include/sgl/audio/AudioI.hpp
  - engine/audio/include/sgl/audio/NullAudio.hpp
  - engine/audio/include/sgl/audio/ScaledAudio.hpp
  - engine/audio/include/sgl/audio/Tone.hpp
  - engine/audio/include/sgl/audio/Blip.hpp
  - engine/audio/src/Tone.cpp
  - engine/audio/src/Blip.cpp
  - engine/audio/src/NullAudio.cpp
  - engine/audio/src/AudioI.cpp
  - engine/audio/CMakeLists.txt
  - engine/sfml/include/sgl/sfml/SfmlAudio.hpp
  - engine/sfml/src/SfmlAudio.cpp
  - tests/engine/audio/ToneTest.cpp
  - tests/engine/audio/BlipTest.cpp
  - tests/engine/audio/ScaledAudioTest.cpp
  - tests/mocks/AudioSpy.hpp
related_docs:
  - source-layout.md
  - engine-resources.md
  - engine-core.md
  - ../explanation/architecture.md
  - ../how-to/build-and-test.md
keywords: [sgl, audio, AudioI, NullAudio, ScaledAudio, SfmlAudio, SoundId, Pcm, tone, blip, Waveform, Envelope]
last_reviewed: 2026-09-29
---

# Engine audio

Headless PCM and playback port in `namespace sgl`, included as `<sgl/audio/X.hpp>`. Target `sgl_audio` is a STATIC library that PUBLIC-links `sgl_core`. No SFML dependency.

SFML playback lives in `sgl::sfml::SfmlAudio` (`sgl_sfml`, links `SFML::Audio` PRIVATE).

## SoundId and Pcm

- `SoundId` — `Handle<SoundTag>` (same style as `TextureId`)
- `Pcm` — mono buffer: `uint32_t sampleRate` + `std::vector<int16_t> samples`

## Waveform, Envelope, tone

`enum class Waveform { Square, Triangle, Noise }`.

`Envelope` holds `Seconds attack` and `Seconds release`. Linear fades: attack from silence to full, release from full to silence; sustain between them is full gain.

`tone(waveform, freqHz, duration, sampleRate, envelope, amplitude)` builds mono PCM. Sample count is `duration * sampleRate`. Peak magnitude never exceeds `amplitude` (scaled to int16). Noise samples use `sgl::Pcg32` from `<sgl/core/Random.hpp>`.

## blip

`blip(hz, seconds, waveform, gain)` is a short UI/SFX helper over `tone`: sample rate `22050`, envelope attack `0.005s` / release `0.03s`, amplitude `gain`.

## AudioI and NullAudio

```cpp
class AudioI {
public:
    virtual ~AudioI();
    virtual void play(SoundId, float volume) = 0;
};
```

`NullAudio` implements `play` as a no-op (any id / volume accepted). Tests use `AudioSpy` (`tests/mocks/AudioSpy.hpp`) to record `{SoundId, volume}` plays.

## ScaledAudio

Header-only wrapper (`ScaledAudio.hpp`, no `.cpp`). Decorates an inner `AudioI&`:

| API | Behavior |
|-----|----------|
| `ScaledAudio(AudioI& inner)` | Stores a reference; does not own |
| `setMaster(float volume01)` | Clamps to `[0,1]` |
| `play(id, volume)` | Forwards `inner.play(id, volume * master_)` |

Use to apply a persisted master volume without changing `AudioI::play`'s signature. Scenes still call `play` on the scaled facade; they do not touch the platform.

## SfmlAudio

`SfmlAudio : AudioI` caches `sf::SoundBuffer` via `ResourceCache<SoundId, sf::SoundBuffer>`.

| API | Behavior |
|-----|----------|
| `upload(key, pcm)` | Load/cache buffer; empty or zero-rate PCM → `DecodeFailed` |
| `play(id, volume)` | Volume in `[0,1]` mapped to SFML 0–100. Picks a stopped voice among 16 `std::optional<sf::Sound>` slots; if all busy, replaces the oldest started |

`SfmlPlatform` owns `SfmlAudio` via `std::unique_ptr` (member order: window, assets, audio, renderer) so the platform header exposes only `AudioI& audio()` without pulling SFML Audio into game TUs. Links `SFML::Audio` PRIVATE.
