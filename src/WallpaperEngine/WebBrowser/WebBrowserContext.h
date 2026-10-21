#pragma once

#include "WallpaperEngine/Application/ApplicationContext.h"
#include "WallpaperEngine/Application/WallpaperApplication.h"

// macOS port: web wallpapers (CEF) are not supported, the context is an empty shell.
namespace WallpaperEngine::WebBrowser {
class WebBrowserContext {
public:
    explicit WebBrowserContext (WallpaperEngine::Application::WallpaperApplication&) { }
};
} // namespace WallpaperEngine::WebBrowser
