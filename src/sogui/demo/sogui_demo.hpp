#pragma once

#include <imgui.h>

namespace SoGui {

inline void ShowDemoWindow(bool* open = nullptr) {
    if (!ImGui::Begin("So-gui demo", open)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
    ImGui::End();
}

} // namespace SoGui
