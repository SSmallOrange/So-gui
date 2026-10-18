#include <sogui/demo/sogui_demo.hpp>

#include <imgui.h>

namespace SoGui {

void ShowDemoWindow(bool* open) {
    if (!ImGui::Begin("So-gui demo", open)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
    ImGui::End();
}

} // namespace SoGui
