#pragma once

#include <sogui/sogui.h>

#include <atomic>

namespace SoGui {

struct Context {
    unsigned long long frameCount = 0;
    std::atomic<bool>  redrawRequested{false};
};

} // namespace SoGui
