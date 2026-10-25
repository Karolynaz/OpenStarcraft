#pragma once
namespace port3ds {
// Native sfxdata IDs occupy the original range. Private mission WAV strings
// use a separate range; the bank stores both without name/path lookup on 3DS.
constexpr unsigned campaign_sound_base=1200, audio_sound_limit=2300;
}
