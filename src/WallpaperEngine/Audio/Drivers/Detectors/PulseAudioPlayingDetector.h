#pragma once

#include "AudioPlayingDetector.h"

// macOS port: nothing to detect, never "something else is playing".
namespace WallpaperEngine::Audio::Drivers::Detectors {
class PulseAudioPlayingDetector final : public AudioPlayingDetector {
public:
    explicit PulseAudioPlayingDetector (
	Application::ApplicationContext& appContext, const Render::Drivers::Detectors::FullScreenDetector& detector
    ) : AudioPlayingDetector (appContext, detector) { }
    ~PulseAudioPlayingDetector () override = default;
    void update () override { }
};
} // namespace WallpaperEngine::Audio::Drivers::Detectors
