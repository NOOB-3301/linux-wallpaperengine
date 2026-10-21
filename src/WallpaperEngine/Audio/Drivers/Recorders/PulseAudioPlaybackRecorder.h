#pragma once

#include "PlaybackRecorder.h"

// macOS port: no PulseAudio, audio-responsive data stays at zero.
namespace WallpaperEngine::Audio::Drivers::Recorders {
class PulseAudioPlaybackRecorder final : public PlaybackRecorder {
public:
    PulseAudioPlaybackRecorder () = default;
    ~PulseAudioPlaybackRecorder () override = default;
    void update () override { }
};
} // namespace WallpaperEngine::Audio::Drivers::Recorders
