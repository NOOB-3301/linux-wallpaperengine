#pragma once

#include "MediaSource.h"

// macOS port: no DBus/MPRIS, so there is never any media info.
namespace WallpaperEngine::Media {
class DBusMediaSource : public MediaSource {
public:
    explicit DBusMediaSource (std::chrono::milliseconds updateInterval) : MediaSource (updateInterval) { }
    ~DBusMediaSource () override = default;
    void update () override { }

protected:
    void performUpdate () override { }
};
}
