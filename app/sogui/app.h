#pragma once

#include <functional>

namespace SoGui {

struct AppConfig {
    const char* title = "So-gui";
    int         width = 1280;
    int         height = 800;
    float       idleFps = 0.0f; // 0：空闲时完全阻塞，只在有事件时重绘
    std::function<void()> onInit;
    std::function<void()> onFrame;
    std::function<void()> onShutdown;
};

int RunApp(const AppConfig& config);

} // namespace SoGui
